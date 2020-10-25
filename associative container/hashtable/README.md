# Hash Table

## 概念

hash是一种经典的空间换时间的策略

RB_Tree消耗的内存是线性的，操作时间是log

考虑这样一个问题：

    现在有n个数字，大小介于[0,65536],判断n个数字里是否包含x

如果使用set（RB_tree）的话，不考虑建表，单次查询时间是logn

如果使用一个 长度为 66536 的数组，我们完全可以o(1)的判断

但是，数据的大小范围很大的话，比如int范围，我们不可能用这么大的内存，这时候就需要hash，来讲元素映射到一个可以解释的索引范围，用于映射的函数就是我们说的hash function（散列函数）

使用hash会带来一个问题，不同的元素可能会映射到一个索引上，这就是hash冲突

## 解决hash冲突的方式

线性探测、二次探测 怪怪的，本质尝试塞满一个数组，然后快满的时候扩容，数据少的时候还行，大的时候不太行

开链：一维数组，数组里每个元素是一个链表（list），这样hash值相同的会放到一个链表里，只需要在这个链表里找即可

## hashtable 节点

用的开链法

* 桶 buckets：用 vector 表示
* hash table 节点：链表的节点

对于一个数，进行散列函数处理，获得一个索引(就是桶的节点)；当数量很大时，经过散列函数处理，会得到相同的索引，那么桶的节点位置一样。

```cpp
// hash table 节点
template <class _Val>
struct _Hashtable_node
{
  _Hashtable_node* _M_next; //没有采用stl的list 为什么？ 太费内存了吧
  _Val _M_val;
};  
```

## hashtable迭代器

hashtable迭代器相当于是一个二维数组遍历

需要记录当前的桶和节点

遍历还蛮简单的，只需要调用_Hashtable_node.next即可，如果到链表尾部的话跳转到下一个不为空的桶

```cpp
// hash table 迭代器
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator {
    typedef hashtable<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc> _Hashtable;
    typedef _Hashtable_node<_Val> _Node; // hash table 节点

    typedef forward_iterator_tag iterator_category; // 迭代器类型：前向迭代器

    _Node* _M_cur; // 迭代器目前所指的节点
    _Hashtable* _M_ht; // 当前所在的桶
    ...
};


template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  _M_cur = _M_cur->_M_next;
  if (!_M_cur) { // 当前链表为尾端，此时选择下一个 不为空的bucket 节点
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}

```


## hashtable 数据结构


```cpp
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
    ...
  hasher                _M_hash;  // hash 函数
  key_equal             _M_equals; // 判断相等
  _ExtractKey           _M_get_key; // 从节点中取出键值
  vector<_Node*,_Alloc> _M_buckets; // vector 容器，也就是 桶
  size_type             _M_num_elements;
    ...
}
```

桶的大小可以为任意值，但是stl默认采用质数大小来设计buckets

可以减少碰撞几率，原始数据不大会是真正的随机的，可能有某些规律，比如大部分是偶数，这时候如果HASH数组容量是偶数，容易使原始数据HASH后不会均匀分布。
比如 2 4 6 8 10 12这6个数，如果对 6 取余 得到 2 4 0 2 4 0 只会得到3种HASH值，冲突会很多 。如果对 7 取余 得到 2 4 6 1 3 5 得到6种HASH值，没有冲突

同样地，如果数据都是3的倍数，而HASH数组容量是3的倍数，HASH后也容易有冲突，用一个质数则会减少冲突的概率

```cpp
enum { __stl_num_primes = 28 };
// SGI STL 提供 28 个质数，用质数大小来设计 buckets
static const unsigned long __stl_prime_list[__stl_num_primes] =
{
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};

// 找大于等于n的下一个质数
inline unsigned long __stl_next_prime(unsigned long __n)
{
  const unsigned long* __first = __stl_prime_list;
  const unsigned long* __last = __stl_prime_list + (int)__stl_num_primes;
  const unsigned long* pos = lower_bound(__first, __last, __n);
  return pos == __last ? *(__last - 1) : *pos;
}
```

## 插入操作

和rb_tree一样支持两种插入

```cpp
  // 不允许有重复的节点
  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    resize(_M_num_elements + 1); // 判断是否需要重置 buckets
    return insert_unique_noresize(__obj);
  }

  // 允许有重复的节点
  iterator insert_equal(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_equal_noresize(__obj);
  }

  // 在不需要重建 buckets 大小下，插入新节点，键值不能重复
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
pair<typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator, bool> 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj); // 计算应该被分配到哪个桶里
  _Node* __first = _M_buckets[__n];

 // 遍历一遍桶里的元素
  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) 
      return pair<iterator, bool>(iterator(__cur, this), false);  // 说明插入节点已经在 hash table 中，不用插入

  _Node* __tmp = _M_new_node(__obj); // 头插法
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return pair<iterator, bool>(iterator(__tmp, this), true);
}

// 在不需要重建 buckets 大小下，插入新节点，键值可以重复
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      _Node* __tmp = _M_new_node(__obj); // 相等，插入后面
      __tmp->_M_next = __cur->_M_next; 
      __cur->_M_next = __tmp;
      ++_M_num_elements;
      return iterator(__tmp, this);
    }

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return iterator(__tmp, this);
}
```


还蛮神秘的，如果元素个数(插入后)大于桶的个数就重建buckets

```cpp
// 重建 buckets
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::resize(size_type __num_elements_hint)
{
  const size_type __old_n = _M_buckets.size();

  if (__num_elements_hint > __old_n) { 
    const size_type __n = _M_next_size(__num_elements_hint); // 找到下一个质数
    if (__n > __old_n) {
      vector<_Node*, _All> __tmp(__n, (_Node*)(0),
                                 _M_buckets.get_allocator()); // 设置新的 buckets
      __STL_TRY {
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = _M_buckets[__bucket];
          while (__first) {// 变量一个桶里的链表
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n); // 计算被重新分配到哪个桶里面
            // 这里采用头插法，即在头部插入删除，比尾插插入更快
            _M_buckets[__bucket] = __first->_M_next;  
            __first->_M_next = __tmp[__new_bucket]; 
            __tmp[__new_bucket] = __first;
            __first = _M_buckets[__bucket];          
          }
        }
        _M_buckets.swap(__tmp); // 新旧对调
      }
#         ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = __tmp[__bucket]->_M_next;
            _M_delete_node(__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
    }
  }
}
```

## 清空和拷贝

因为桶里存的是链表，所以clear和拷贝的时候需要注意内存的分配

```cpp
// 清空所有节点
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::clear()
{
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = _M_buckets[__i];
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
    _M_buckets[__i] = 0;
  }
  _M_num_elements = 0;
}

// 哈希表复制，第一：vector 复制，第二：linked list 复制   
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_copy_from(const hashtable& __ht)
{
  _M_buckets.clear(); // 先清空当前的
  _M_buckets.reserve(__ht._M_buckets.size()); //分配空间，当前大的话不变，小的话会扩张
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (_Node*) 0);
  __STL_TRY {
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
      const _Node* __cur = __ht._M_buckets[__i];
      if (__cur) {
        _Node* __copy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __copy;

        for (_Node* __next = __cur->_M_next; 
             __next; 
             __cur = __next, __next = __cur->_M_next) {
          __copy->_M_next = _M_new_node(__next->_M_val);
          __copy = __copy->_M_next;
        }
      }
    }
    _M_num_elements = __ht._M_num_elements;
  }
  __STL_UNWIND(clear());
}

```

## hash函数

stl默认提供了一些hash函数。其实只有string进行了hash，其他的都没变（最后会模bucket.size）

c++11应该提供了更多,比如这里没有std::string

```cpp
// 仿函数声明
template <class _Key> struct hash { }; 

// 对字符字符串进行转换
inline size_t __stl_hash_string(const char* __s)
{
  unsigned long __h = 0; 
  for ( ; *__s; ++__s)
    __h = 5*__h + *__s;
  
  return size_t(__h);
}

// 下面都是偏特化
__STL_TEMPLATE_NULL struct hash<char*>
{
  size_t operator()(const char* __s) const { return __stl_hash_string(__s); }
};

__STL_TEMPLATE_NULL struct hash<const char*>
{
  size_t operator()(const char* __s) const { return __stl_hash_string(__s); }
};

// 其它类型，直接返回原值
__STL_TEMPLATE_NULL struct hash<char> {
  size_t operator()(char __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<unsigned char> {
  size_t operator()(unsigned char __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<signed char> {
  size_t operator()(unsigned char __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<short> {
  size_t operator()(short __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<unsigned short> {
  size_t operator()(unsigned short __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<int> {
  size_t operator()(int __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<unsigned int> {
  size_t operator()(unsigned int __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<long> {
  size_t operator()(long __x) const { return __x; }
};
__STL_TEMPLATE_NULL struct hash<unsigned long> {
  size_t operator()(unsigned long __x) const { return __x; }
};
```

总的来说还蛮简单的，比rb tree……