## 基本allocator

```cpp
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
    // ::operator new表示全局命名空间下的operator new
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

```

好多神必操作

上面的代码只是operator new和operator delete的简单封装

真正的stl代码还是做了很多优化

## cpp内存配置

```cpp
class Foo{};

Foo* pf = new Foo();
delete pf;
```

new 事实上有两个阶段的操作：

1. 调用::opreator new配置内存、
2. 调用Foo::Foo()构造对象内容

delete 事实上也有两个阶段的操作：

1. 调用Foo::~Foo()将对象析构
2. 调用::opreator delete释放内存

## construct和destroy

construct和destroy主要是负责对象的构造和析构两个过程

construct没啥优化的，还是一个泛型函数，封装了一下placement new

对于destroy，有两种，一种是单独一个指针，直接调用析构函数即可。另一种是指定区间，对多个对象进行destroy，这里有一个优化，如果对象是trivial destructor(类中只有基本的数据类型,析构函数基本没有什么用)，那么什么也不做，如果对象是non-trivial destructor，则循环遍历，一个一个析构。

注：这里为了判断对象的类型，还要用到type trait技术，在第三章讲

## 空间的配置与释放，std::alloc(SGI独有)

SGI对此的设计哲学是：

1. 向system heap要求空间
2. 考虑多线程(书上略过了这部分)
3. 考虑内存不足时的应对措施
4. 考虑过多小型模块可能造成的内存碎片问题

为了避免内存碎片问题，SGI设计了双层级配置器：第一级直接调用malloc和free；第二层级考虑不同情况：当申请内存超过128byte，视为足够大，调用第一级；当内存小于128byte时，视为足够小，使用内存池

![第一级和第二级配置器](https://s1.ax1x.com/2020/04/20/J1FBO1.md.png)
![配置器接口包装方式](https://s1.ax1x.com/2020/04/20/J1FOpQ.png)
