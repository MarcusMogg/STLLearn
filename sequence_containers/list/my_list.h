#include "../../allocator/my_alloc.h"
#include "../../allocator/my_construct.h"

#ifndef _MY_LIST_H_
#define _MY_LIST_H_
namespace MOG
{
template <class T>
struct list_node
{
    typedef list_node<T> *list_node_pt;
    list_node_pt prev;
    list_node_pt next;
    T data;
};

template <class T, class Ref, class Ptr>
struct list_iterator
{
    typedef list_iterator<T, Ref, Ptr> self;
    typedef list_iterator<T, T &, T *> iterator;

    // typedef bidirectional_iterator_tag iterator_category;
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef list_node<T> *link_type;
    typedef size_t size_type;
    typedef ptrdiff_t different_type;

    link_type node;

    list_iterator() {}
    list_iterator(link_type x) : node(x) {}
    list_iterator(const list_iterator &x) : node(x.node) {}

    bool operator==(const self &x) const { return x.node == node; }
    bool operator!=(const self &x) const { return x.node != node; }

    // 这几个重载有点意思
    reference operator*() const { return (*node).data; }
    pointer operator->() const { return &(operator*()); }
    // ++i
    self &operator++()
    {
        node = node->next;
        return *this;
    }
    // i++
    self operator++(int)
    {
        self tmp = *this;
        ++*this; //这里是调用了上面那个函数不是指针+
        return tmp;
    }
    self &operator--()
    {
        node = node->prev;
        return *this;
    }
    self operator--(int)
    {
        self tmp = *this;
        --*this;
        return tmp;
    }
};

// list是一个环状双向链表
template <class T, class Alloc = default_alloc>
class my_list
{
public:
    typedef list_node<T> node_type;
    typedef node_type *link_type;
    typedef list_iterator<T, T &, T *> iterator;
    // 一个指针就可以表示环状双向链表
    // node 指向尾端的一个空白指针
    node_type node;

    my_list(/* args */);
    ~my_list();

    iterator begin() { return node->next; }
    iterator end() { return node; }
    bool empty() const { return node->next == node; }

    // 将x插入到pos前
    iterator insert(iterator pos, const T &x)
    {
        link_type tmp; //= create_node(x); 分配一个node内存
        tmp->next = pos.node;
        tmp->prev = pos.node->prev;
        pos.node->prev->next = tmp;
        pos.node->prev = tmp;
        return tmp;
    }
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
};
} // namespace MOG
#endif