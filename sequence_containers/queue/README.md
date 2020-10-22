## deque 
### deque 概述

deque 是双向开口的连续线性空间

对比vector, push_back的时候, deque和vector差不多；但是push_front 的时候，vector的效率很差(因为要拷贝数据)

最大的差异：

1. deque头部插入是O(1)的，只能从头部和尾部读取插入
2. deque没有容量的概念，是由多段连续空间动态组成的，扩充内存的时候不会去复制元素到新的空间(这会导致[]操作很复杂，但是deque为什么要支持random access（逃）)

### deque 中控器

因为 deque 是一段段的定量连续空间构成。为了方便管理这些一段段连续空间，同时做到对用户来说，deque 整体是连续的，所以需要采用一小块连续空间 map(不是stl里面那个map,对于deque<T>, map相当于是T**) 作为控制单元，其中 map 中的每个节点 node 都是指针，指向 deque 的一段连续空间 buffer 缓冲区。

SGI STL 允许指定 buffer 的大小，默认值为 0 表示将使用 512 bytes 缓冲区。

![map的结构](https://s1.ax1x.com/2020/10/21/BCckmd.png)

扩张时只扩充map，不会改变buffer区

### deque迭代器

deque的迭代器++ --操作比vector要复杂一点，因为可能存在到达一段空间结尾时跳转到其他空间的情况。所以迭代器需要知道：1.当前行 2. 当前元素的位置 3. 当前行的开始和结尾

核心的几个函数

```cpp
template <class T, class Ref, class Ptr, size_t BufSiz = 0>
class my_deque_iterator
{
    T *cur;
    T *first;
    T *last;
    map_pointer map;
    
    self &operator++();
    self &operator--();

    self &operator+=(difference_type __n);
    self operator+(difference_type __n) const;
}
```

### deque内存管理

deque本身的数据结构没什么说的，存一下开始和结尾两个迭代器，基本都可以考迭代器的实现

麻烦的是内存分配

map部分的分配其实核心和vector有部分相同，会有预备空间，开始时start和finish在中间位置，然后向两端扩张，一端满了之后重新分配(乘2)然后拷贝。特别的是，考虑现在已申请的空间大于已用空间二倍大小的情况（一端一直push，另一端一直pop），这时候不用重新分配，把数据重新拷贝回中间即可

所以deque的迭代器也是会失效的？

扩张后缓冲区是不会变的

pop_back和pop_front的时候，如果缓冲区为空会被释放

(代码太多不想抄QAQ)

## stack和queue

stack和queue基本就是把deque封装一下，只用部分接口

有点像设计模式里面的适配器模式

stack和queue没有迭代器

