#pragma once
#ifndef _MOG_ALLOC_H_
#define _MOG_ALLOC_H_

#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifndef __THROW_BAD_ALLOC
#if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#include <stdio.h>

#include <cstdlib>
#define __THROW_BAD_ALLOC               \
    fprintf(stderr, "out of memory\n"); \
    exit(1)
#else /* Standard conforming out-of-memory handling */
#include <new>
#define __THROW_BAD_ALLOC throw std::bad_alloc()
#endif
#endif

namespace MOG
{

// 单纯地转调用，调用传递给配置器(第一级或第二级)；多一层包装，使 _Alloc
// 具备标准接口
template <class _Tp, class _Alloc>
class simple_alloc
{
public:
    // 配置 n 个元素
    static _Tp *allocate(size_t __n)
    {
        return 0 == __n ? nullptr
                        : static_cast<_Tp *>(_Alloc::allocate(__n * sizeof(_Tp)));
    }
    static _Tp *allocate(void)
    {
        return static_cast<_Tp *>(_Alloc::allocate(sizeof(_Tp)));
    }
    static void deallocate(_Tp *__p, size_t __n)
    {
        if (0 != __n)
            _Alloc::deallocate(__p, __n * sizeof(_Tp));
    }
    static void deallocate(_Tp *__p)
    {
        _Alloc::deallocate(__p, sizeof(_Tp));
    }
};

class malloc_alloc
{
private:
    // oom ： out of memory
    static void *_S_oom_malloc(size_t);
    static void *_S_oom_realloc(void *, size_t);
    // 类似于operator new 的new handle
    typedef void(handler)();
    static handler *__malloc_alloc_oom_handler;
    static int test;

public:
    // 第一级配置器直接调用 malloc()
    static void *allocate(size_t n)
    {
        void *result = malloc(n);
        if (nullptr == result) // 内存满了
        {
            result = _S_oom_malloc(n);
        }
        return result;
    }

    // 第一级配置器直接调用 free()
    static void deallocate(void *p, size_t /* __n */)
    {
        free(p);
    }

    // 第一级配置器直接调用 realloc()
    static void *reallocate(void *p, size_t old_sz, size_t new_sz)
    {
        void *result = realloc(p, new_sz);
        if (nullptr == result)
        {
            result = _S_oom_realloc(p, new_sz);
        }
        return result;
    }

    // 以下仿真 C++ 的 set_new_handler()，可以通过它指定自己的 out-of-memory
    // handler 为什么不使用 C++ new-handler 机制，因为第一级配置器并没有
    // ::operator new 来配置内存 为什么不用::operator
    // new，历史原因？或者没有realloc？不懂
    static handler *__set_malloc_handler(handler *f)
    {
        handler *old = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return old;
    }
};

malloc_alloc::handler *malloc_alloc::__malloc_alloc_oom_handler = nullptr;

inline void *malloc_alloc::_S_oom_malloc(size_t n)
{
    for (;;)
    {
        malloc_alloc::handler *my_malloc_handler = __malloc_alloc_oom_handler;
        if (nullptr == my_malloc_handler)
        {
            __THROW_BAD_ALLOC;
        }
        my_malloc_handler();
        void *result = malloc(n);
        if (result)
            return result;
    }
}

inline void *malloc_alloc::_S_oom_realloc(void *p, size_t n)
{
    for (;;)
    {
        malloc_alloc::handler *my_malloc_handler = __malloc_alloc_oom_handler;
        if (nullptr == my_malloc_handler)
        {
            __THROW_BAD_ALLOC;
        }
        my_malloc_handler();
        void *result = realloc(p, n);
        if (result)
            return result;
    }
}

class default_alloc
{
private:
    static const int ALIGN = 8;
    static const int MAX_BYTES = 128;
    static const int N_FREELISTS = MAX_BYTES / ALIGN;
    static size_t round_up(size_t bytes)
    {
        return (bytes + ALIGN - 1) & ~(ALIGN - 1);
    }
    union obj
    {
        union obj *nxt;
        char data[1];
    };
    static obj *free_list[N_FREELISTS];
    static size_t freelist_index(size_t bytes)
    {
        return (bytes - 1) / (size_t)ALIGN;
    }
    //从内存池里挖碎片
    static void *refill(size_t n);
    //内存池
    static char *chunk_alloc(size_t size, int &nobjs);

    static char *start_free; // 内存池起始位置。只在 _S_chunk_alloc() 中变化
    static char *end_free;   // 内存池结束位置。只在 _S_chunk_alloc() 中变化
    static size_t heap_size; // 内存池大小

public:
    static void *allocate(size_t n)
    {
        if (n > MAX_BYTES)
            return malloc_alloc::allocate(n);
        obj **my_free_list = free_list + freelist_index(n);
        obj *result = *my_free_list;
        if (nullptr == result)
        {
            return refill(round_up(n));
        }
        *my_free_list = result->nxt;
        return result;
    }
    static void deallocate(void *p, size_t n)
    {
        if (n > MAX_BYTES)
            return malloc_alloc::deallocate(p, n);
        obj **my_free_list = free_list + freelist_index(n);
        obj *front = static_cast<obj *>(p); // 归还内存，插到链表头部
        front->nxt = *my_free_list;
        *my_free_list = front;
    }
    // 把原来的放回内存池，然后申请一个新的
    static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

inline void *default_alloc::refill(size_t n)
{
    int nobjs = 20;
    char *chunk = chunk_alloc(n, nobjs);
    if (1 == nobjs)
        return chunk;

    obj **my_free_list = free_list + freelist_index(n);
    obj *result = reinterpret_cast<obj *>(chunk);
    obj *cur, *nxt;
    *my_free_list = nxt = reinterpret_cast<obj *>(chunk + n);
    for (int i = 1; i < nobjs; ++i)
    {
        cur = nxt;
        nxt = reinterpret_cast<obj *>(reinterpret_cast<char *>(nxt) + n);
        if (i == nobjs - 1)
        {
            cur->nxt = nullptr;
        }
        else
        {
            cur->nxt = nxt;
        }
    }
    return result;
}

inline char *default_alloc::chunk_alloc(size_t size, int &nobjs)
{
    char *result;
    size_t total_bytes = size * nobjs;
    const size_t left_bytes = end_free - start_free;

    // 剩余内存足够
    if (left_bytes >= total_bytes)
    {
        result = start_free;
        start_free += total_bytes;
        return result;
    }
    // 剩余内存不足，但是够至少一个
    if (left_bytes >= size)
    {
        nobjs = left_bytes / size;
        total_bytes = nobjs * size;
        result = start_free;
        start_free += total_bytes;
        return result;
    }
    // 剩余内存太少，一个都不够
    size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
    if (left_bytes > 0) // 把边边角角的这些内存存起来，不能浪费
    {
        obj **my_free_list = free_list + freelist_index(left_bytes); // 剩余内存一定是八的倍数
        reinterpret_cast<obj *>(start_free)->nxt = *my_free_list;
        *my_free_list = reinterpret_cast<obj *>(start_free)->nxt;
    }
    // 重新申请内存
    start_free = static_cast<char *>(malloc(bytes_to_get));
    if (nullptr == start_free) // 没申请到，内存不够
    {
        // 尝试看free_list里有没有足够大的分出去一点
        for (size_t i = size; i < MAX_BYTES; i += ALIGN)
        {
            obj **my_free_list = free_list + freelist_index(i);
            if (nullptr != *my_free_list)
            {
                start_free = (*my_free_list)->data;
                end_free = start_free + i;
                *my_free_list = (*my_free_list)->nxt;
                // 有内存了去分配
                return chunk_alloc(size, nobjs);
            }
        }
        // free_list也没
        end_free = nullptr;
        // 最后的挣扎，如果还申请不到会throw_bad_alloc
        start_free = static_cast<char *>(malloc_alloc::allocate(bytes_to_get));
    }
    // 申请到，加入内存池
    heap_size += bytes_to_get;
    end_free = start_free + bytes_to_get;
    // 有内存了去分配
    return chunk_alloc(size, nobjs);
}

inline void *default_alloc::reallocate(void *p, size_t old_sz, size_t new_sz)
{
    if (old_sz > MAX_BYTES && new_sz > MAX_BYTES)
        return realloc(p, new_sz);
    if (round_up(old_sz) == round_up(new_sz))
        return p;
    void *result = allocate(new_sz);
    const size_t copy_size = new_sz > old_sz ? old_sz : new_sz;
    memcpy(result, p, copy_size);
    deallocate(p, old_sz);
    return result;
}

default_alloc::obj *default_alloc::free_list[default_alloc::N_FREELISTS] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
char *default_alloc::start_free = nullptr;
char *default_alloc::end_free = nullptr;
size_t default_alloc::heap_size = 0;
} // namespace MOG
#endif
