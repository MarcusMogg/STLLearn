list是环状双向链表

ps:这点在不同编译器里有不同的表现，stl源码剖析是这样说的,但是在vs里面会越界报错,比如
```cpp
    list<int> a = {1,2,3};

    auto k = a.begin();
    ++k;
    ++k;
    ++k;
    ++k;
    cout << *k;
```
在vs里会报错但是g++可以
神必

链表和vector 的区别就不用多说

需要注意的是vector任何改变元素数量的操作都会导致所有迭代器失效（vector可能会copy到一个新的空间，地址改变了）, list不会

核心操作是transfer操作

```cpp
    // 将[first,last)迁移到pos前面
    void transfer(iterator pos, iterator first, iterator last)
    {
        if (pos != last)
        {
            last.node->prev->next = pos;
            first.node->prev->next = last;
            pos.node->prev->next = first;

            iterator tmp = pos.node->prev;
            pos.node->prev = last.node->prev;
            last.node->prev = first.node->prev;
            first.node->prev = tmp;
        }
    }
```

splice就是transfer

merge(就是mergesort那个merge，不过吧数组改成链表)相当与一个链表的一部分不断transfer到另一个链表的相应位置

sort和algorithm里的sort实现不一样，核心思想是mergesort 链表版