#ifndef _MY_ALLOCATOR_
#define _MY_ALLOCATOR_

#include <new>
#include <cstddef>
#include <cstdlib>
#include <climits>
#include <iostream>

namespace MOG
{
template <class T>
inline T *_allocate(ptrdiff_t size, T *)
{
    // 这个函数是用来设置默认内存申请函数(operator new和operator new[])申请内存失败时调用的方法
    // 设为nullptr是让new 不抛出异常（默认是会抛出bad_alloc）
    std::set_new_handler(nullptr);
    T *tmp = static_cast<T *>(::operator new(size * sizeof(T))); // 分配内存

    if (tmp == nullptr)
    {
        std::cerr << "内存不足";
        exit(-1);
    }
    return tmp;
}

template <class T>
inline void _deallocate(T *buff)
{
    ::operator delete(buff);
}

template <class T1, class T2>
inline void _construct(T1 *p, const T2 &v)
{
    // placement new 在p指向的内存区域构造一个T1类型的变量
    new (p) T1(v);
}

template <class T>
inline void _destroy(T *ptr)
{
    ptr->~T(); //显式调用析构函数
}

template <class T>
class allocator
{
public:
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <class U>
    struct rebind
    {
        typedef allocator<U> other;
    };

    pointer allocate(size_type n, const void *hint = nullptr)
    {
        return _allocate(n, static_cast<T *>(nullptr));
    }

    void deallocate(pointer p, size_type n)
    {
        _deallocate(p);
    }

    void construct(pointer p, const T &value)
    {
        _construct(p, value);
    }

    void destroy(pointer p) { _destroy(p); }

    pointer address(reference x) { return &x; }
    const_pointer const_address(const_reference x) { return &x; }

    size_type max_size() const
    {
        return size_type(UINT_MAX / sizeof(T));
    }
};
} // namespace MOG

#endif
