#include "../../allocator/my_alloc.h"
#include "../../allocator/my_construct.h"

#ifndef _MY_DEQUE_H_
#define _MY_DEQUE_H_
namespace MOG
{
// 决定缓冲区大小的函数，全局函数，当 __size 小于 512 bytes，就返回 512 / __size
inline size_t __deque_buf_size(size_t __size)
{
    return __size < 512 ? size_t(512 / __size) : size_t(1);
}

template <class T, class Ref, class Ptr, size_t BufSiz = 0>
class my_deque_iterator
{
public:
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T **map_pointer;
    typedef my_deque_iterator<T, Ref, Ptr> self;

    T *cur;           //当前元素
    T *first;         // 当前行开始
    T *last;          // 当前行结尾
    map_pointer node; // 当前行

    static size_t _S_buffer_size() { return __deque_buf_size(sizeof(T)); }

    reference operator*() const { return *cur; }
    pointer operator->() const
    {
        return &(operator*());
    }

    difference_type operator-(const self &__x) const
    {
        return difference_type(_S_buffer_size()) * (node - __x.node - 1) +
               (cur - first) + (__x.last - __x.cur);
    }
    void set_node(map_pointer __new_node)
    {
        node = __new_node;
        first = *__new_node;
        last = first + difference_type(_S_buffer_size());
    }
    self &operator++() // 切换到下一个段
    {
        ++cur;
        if (cur == last)
        {
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }
    self operator++(int)
    {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self &operator--() // 切换到上一个段
    {
        if (cur == first)
        {
            set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }
    self operator--(int)
    {
        self tmp = *this;
        --*this;
        return tmp;
    }

    self &operator+=(difference_type __n)
    {
        difference_type __offset = __n + (cur - first);
        if (__offset >= 0 && __offset < difference_type(_S_buffer_size()))
            cur += __n;
        else
        {
            difference_type __node_offset =
                __offset > 0 ? __offset / difference_type(_S_buffer_size())
                             : -difference_type((-__offset - 1) / _S_buffer_size()) - 1;
            set_node(node + __node_offset);
            cur = first +
                  (__offset - __node_offset * difference_type(_S_buffer_size()));
        }
        return *this;
    }

    //More Effective C++ 第22 条, 用复合=的op实现简单op。有点神必哦
    // 考虑 x = x + y 和 x += y
    self operator+(difference_type __n) const
    {
        self __tmp = *this;
        return __tmp += __n;
    }

    self &operator-=(difference_type __n) { return *this += -__n; }

    self operator-(difference_type __n) const
    {
        self __tmp = *this;
        return __tmp -= __n;
    }

    // 迭代器可以直接移动到 n 个距离，获取该位置的元素
    reference operator[](difference_type __n) const { return *(*this + __n); }
};

template <class T, class Alloc = default_alloc, size_t BufSiz = 0>
class my_deque
{
public:
    typedef T value_type;
    typedef T &reference;
    typedef value_type *pointer;
    typedef pointer *map_pointer;

    // 中控器
    map_pointer map;
    size_t map_size; // map可以容纳多少指针

    typedef my_deque_iterator<T, T &, T *> iterator;

    iterator start;
    iterator finish;

    iterator begin() { return start; }
    iterator end() { return finish; }

    reference operator[](size_t n) { return start[n]; }
};
} // namespace MOG
#endif