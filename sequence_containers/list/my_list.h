#ifndef _MY_LIST_H_
#define _MY_LIST_H_

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
    typedef list_node<T> link_type;
};

#endif