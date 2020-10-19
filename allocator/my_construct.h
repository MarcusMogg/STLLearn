#pragma once
#ifndef _MOG_CONSTRUCT_H_
#define _MOG_CONSTRUCT_H_

#include <new> // for placement new
namespace MOG
{
// construct 就是placement new的简单封装
template <class T1, class T2>
inline void _Construct(T1 *p1, const T2 &p2)
{
    new (static_cast<void *>(p1)) T1(p2);
}

template <class T1>
inline void _Construct(T1 *p1)
{
    new (static_cast<void *>(p1)) T1();
}

template <class T1, class T2>
inline void construct(T1 *p, const T2 &value)
{
    _Construct(p, value);
}

template <class T1>
inline void construct(T1 *p)
{
    _Construct(p);
}

// destroy的优化没写，type traits在后面
template <class T1>
inline void _Destroy(T1 *p)
{
    p->~T1(); //显示调用析构函数
}

template <class T1>
inline void destroy(T1 *p)
{
    _Destroy(p);
}

template <class ForwardIterator>
inline void _Destroy(ForwardIterator first, ForwardIterator last)
{
    for (; first != last; ++first)
    {
        destroy(&(*first));
    }
}
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
    _Destroy(first, last);
}
} // namespace MOG
#endif
