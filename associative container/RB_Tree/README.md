# 红黑树

## 原理

RB-tree是一种平衡二叉搜索树树，满足以下条件：

1. 每个节点不是红色就是黑色
2. 根节点为黑色
3. 如果节点为红色，其子节点为黑色
4. 从任一节点到其每个叶子（null节点）的所有简单路径都包含相同数目的黑色节点
（为了方便，视null节点为黑节点）

## RB_Tree节点

```cpp
typedef bool _Rb_tree_Color_type;
const _Rb_tree_Color_type _S_rb_tree_red = false;  // 红色为 0
const _Rb_tree_Color_type _S_rb_tree_black = true; // 黑色为 1

// RB-tree 节点-基类
struct _Rb_tree_node_base
{
    typedef _Rb_tree_Color_type _Color_type;
    typedef _Rb_tree_node_base *_Base_ptr;

    _Color_type _M_color; // 节点颜色，非红即黑
    _Base_ptr _M_parent;  // 父节点
    _Base_ptr _M_left;    // 左节点
    _Base_ptr _M_right;   // 右节点

    // 找到 RB-tree 的最小值节点,根据二叉搜索树的性质一直往左孩子找即可
    static _Base_ptr _S_minimum(_Base_ptr __x)
    {
        while (__x->_M_left != 0)
            __x = __x->_M_left;
        return __x;
    }

    // 找到 RB-tree 的最大值节点,根据二叉搜索树的性质一直往右孩子找即可
    static _Base_ptr _S_maximum(_Base_ptr __x)
    {
        while (__x->_M_right != 0)
            __x = __x->_M_right;
        return __x;
    }
};

// RB-tree 节点
template <class _Value>
struct _Rb_tree_node : public _Rb_tree_node_base
{
    typedef _Rb_tree_node<_Value> *_Link_type;
    _Value _M_value_field; // 节点值
};
```

## RB_Tree迭代器

RB_Tree迭代器的遍历是按照大小顺序遍历的，因为是一个二叉搜索树，其实比较麻烦

```cpp
// 供 operator++() 调用
void _M_increment()
{
    if (_M_node->_M_right != 0)
    { // 如果有右子节点，就向右走
        _M_node = _M_node->_M_right;
        while (_M_node->_M_left != 0) // 然后一直往左子树走到底
            _M_node = _M_node->_M_left;
    }
    else
    {
        // 没有右子节点，左孩子一定已经找完了，所以往父节点走
        _Base_ptr __y = _M_node->_M_parent; // 没有右子节点，找其父节点
        while (_M_node == __y->_M_right)
        { // 当该节点为其父节点的右子节点，就一直向上找父节点
            _M_node = __y;
            __y = __y->_M_parent;
        }
        // 这个是特判一种情况，_M_node为根节点，且无右节点
        // 这种情况下 header的right是root
        if (_M_node->_M_right != __y)
            _M_node = __y;
    }
}
// 供 operator--() 调用
void _M_decrement()
{ // 红色节点且父节点的父节点等于自己
    // 特例，现在在header结点
    if (_M_node->_M_color == _S_rb_tree_red &&
        _M_node->_M_parent->_M_parent == _M_node)
        _M_node = _M_node->_M_right;
    else if (_M_node->_M_left != 0)
    { // 左子树不为空时，上一个为node 的左子树最大值
        _Base_ptr __y = _M_node->_M_left;
        while (__y->_M_right != 0)
            __y = __y->_M_right;
        _M_node = __y;
    }
    else
    {
        // 左子树为空时，右孩子都已经找过了，往父节点找
        _Base_ptr __y = _M_node->_M_parent;
        while (_M_node == __y->_M_left)
        {
            _M_node = __y;
            __y = __y->_M_parent;
        }
        _M_node = __y;
    }
}
```

## RB_Tree数据结构

核心是三个数据

```cpp
size_type _M_node_count; // 节点数量
_Compare _M_key_compare; // 节点间的键值大小比较方式，是一个仿函数
_Rb_tree_node<_Tp> *_M_header; // RB_tree的header是特殊的，左节点为最小值，右节点为最大值，parent为root
```

header和root会组成一个环(parent)，为了简化边界处理

## 插入操作

RB_Tree支持两种插入操作

这两个函数其实是寻找插入位置

```cpp
    iterator insert_unique(iterator __position, const value_type &__x); // 将 __x 插入到 RB-tree key唯一
    iterator insert_equal(iterator __position, const value_type &__x);  // 将 __x 插入到 RB-tree，允许重复
    // 插入新值，节点键值允许重复
template <class _Key, class _Value, class _KeyOfValue,
          class _Compare, class _Alloc>
typename _Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::iterator
_Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::insert_equal(const _Value &__v)
{
    _Link_type __y = _M_header;
    _Link_type __x = _M_root(); // 从根节点开始
    while (__x != 0)
    {
        __y = __x;
        // 当插入值大于当前节点的值，向右，否则反之
        __x = _M_key_compare(_KeyOfValue()(__v), _S_key(__x)) ? _S_left(__x) : _S_right(__x);
    }
    return _M_insert(__x, __y, __v); // __x 为插入节点，__y 为插入节点的父节点， __v 为插入节点的值
}

// 插入新值，节点键值不允许重复，唯一
template <class _Key, class _Value, class _KeyOfValue,
          class _Compare, class _Alloc>
pair<typename _Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::iterator,
     bool>
_Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::insert_unique(const _Value &__v)
{
    _Link_type __y = _M_header;
    _Link_type __x = _M_root();
    bool __comp = true;
    while (__x != 0)
    {
        __y = __x;
        __comp = _M_key_compare(_KeyOfValue()(__v), _S_key(__x));
        __x = __comp ? _S_left(__x) : _S_right(__x);
    } // 到此和上面一样
    iterator __j = iterator(__y);
    if (__comp) // 离开while的时候__comp为真，当前值v小于j的值将插入左侧
        if (__j == begin())
            return pair<iterator, bool>(_M_insert(__x, __y, __v), true);
        else
            --__j;
    if (_M_key_compare(_S_key(__j._M_node), _KeyOfValue()(__v))) // 反向比较，当前值v大于j的值
        return pair<iterator, bool>(_M_insert(__x, __y, __v), true);
    return pair<iterator, bool>(__j, false); // 重复，与j的可以相同，返回失败
}

```

真正的插入操作

```cpp
// RB-tree 元素插入操作，_KeyOfValue 为仿函数
template <class _Key, class _Value, class _KeyOfValue,
          class _Compare, class _Alloc>
typename _Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::iterator
_Rb_tree<_Key, _Value, _KeyOfValue, _Compare, _Alloc>::_M_insert(_Base_ptr __x_, _Base_ptr __y_, const _Value &__v)
{
    //__x 为插入节点（实际插入位置），__y 为插入节点的父节点， __v 为插入节点的值
    _Link_type __x = (_Link_type)__x_;
    _Link_type __y = (_Link_type)__y_;
    _Link_type __z;

    if (__y == _M_header || __x != 0 || // __x一定为null吧
        _M_key_compare(_KeyOfValue()(__v), _S_key(__y))) // v < _y.value
    {
        __z = _M_create_node(__v);
        _S_left(__y) = __z; // also makes _M_leftmost() = __z
                            //    when __y == _M_header 。和下面不冲突，header只会设置一次left
        if (__y == _M_header) // 特判一下空的情况，其实可以摘出来
        {
            _M_root() = __z;
            _M_rightmost() = __z; // 修改最大值，header->right
        }
        else if (__y == _M_leftmost()) // 修改最小值，header->left
            _M_leftmost() = __z; // maintain _M_leftmost() pointing to min node
    }
    else
    {
        __z = _M_create_node(__v);
        _S_right(__y) = __z;
        if (__y == _M_rightmost())
            _M_rightmost() = __z; // maintain _M_rightmost() pointing to max node
    }
    // 设置父节点
    _S_parent(__z) = __y;
    _S_left(__z) = 0;
    _S_right(__z) = 0;
    // 每次插入可能会破坏红黑树的平衡，需要重新调整
    _Rb_tree_rebalance(__z, _M_header->_M_parent);
    ++_M_node_count;
    return iterator(__z);
}
```

## 调整平衡

每次插入可能会破坏红黑树的平衡，需要重新调整

可以看看这篇博客https://www.cnblogs.com/skywang12345/p/3245399.html

核心思想是：新插入的节点需要是红色（规则4. 从任一节点到其每个叶子（null节点）的所有简单路径都包含相同数目的黑色节点）

这样可能会产生规则3冲突（红的孩子必须是黑）

然后分情况讨论

```cpp
// 左旋转
//     X                               Y
//    / \                             / \ 
//   XL   Y         ------>          X   YR
//       / \        ------>         / \  
//      YL  YR                     XL  YL
inline void
_Rb_tree_rotate_left(_Rb_tree_node_base *__x, _Rb_tree_node_base *&__root)
{
    _Rb_tree_node_base *__y = __x->_M_right;
    __x->_M_right = __y->_M_left;
    if (__y->_M_left != 0)
        __y->_M_left->_M_parent = __x;
    __y->_M_parent = __x->_M_parent;

    // 将x的parent对应的指针指向Y
    if (__x == __root)
        __root = __y;
    else if (__x == __x->_M_parent->_M_left)
        __x->_M_parent->_M_left = __y;
    else
        __x->_M_parent->_M_right = __y;
    __y->_M_left = __x;
    __x->_M_parent = __y;
}

// 右旋转
//     X                               Y
//    / \                             / \ 
//   Y   XR       ------>            YL   X
//  / \           ------>                / \  
// YL  YR                               YR  XR
inline void
_Rb_tree_rotate_right(_Rb_tree_node_base *__x, _Rb_tree_node_base *&__root)
{
    _Rb_tree_node_base *__y = __x->_M_left;
    __x->_M_left = __y->_M_right;
    if (__y->_M_right != 0)
        __y->_M_right->_M_parent = __x;
    __y->_M_parent = __x->_M_parent;

    if (__x == __root)
        __root = __y;
    else if (__x == __x->_M_parent->_M_right)
        __x->_M_parent->_M_right = __y;
    else
        __x->_M_parent->_M_left = __y;
    __y->_M_right = __x;
    __x->_M_parent = __y;
}

// RB-tree 平衡调整
inline void
_Rb_tree_rebalance(_Rb_tree_node_base *__x, _Rb_tree_node_base *&__root)
{
    __x->_M_color = _S_rb_tree_red; // 新节点必为红
    while (__x != __root && __x->_M_parent->_M_color == _S_rb_tree_red) // 父节点为红，违反规则3
    {
        if (__x->_M_parent == __x->_M_parent->_M_parent->_M_left) // 父节点为左子
        {
            _Rb_tree_node_base *__y = __x->_M_parent->_M_parent->_M_right; // 父节点的兄弟节点
            if (__y && __y->_M_color == _S_rb_tree_red)
            {
                // x的祖父节点的两个孩纸都是红，则将两个孩子改为黑，祖父改为红
                // 然后去判断祖父节点是否冲突
                __x->_M_parent->_M_color = _S_rb_tree_black;
                __y->_M_color = _S_rb_tree_black;
                __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
                __x = __x->_M_parent->_M_parent;
            }
            else // null也是是黑节点
            {
                if (__x == __x->_M_parent->_M_right)
                {
                    // x在右侧 先左旋 统一为左侧插入的问题
                    __x = __x->_M_parent;
                    _Rb_tree_rotate_left(__x, __root);
                }
                __x->_M_parent->_M_color = _S_rb_tree_black;
                __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
                _Rb_tree_rotate_right(__x->_M_parent->_M_parent, __root);
            }
        }
        else
        {   // 父节点为右子，基本和上面处理方式一致，左右旋转的方式改变
            _Rb_tree_node_base *__y = __x->_M_parent->_M_parent->_M_left;
            if (__y && __y->_M_color == _S_rb_tree_red)
            {
                __x->_M_parent->_M_color = _S_rb_tree_black;
                __y->_M_color = _S_rb_tree_black;
                __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
                __x = __x->_M_parent->_M_parent;
            }
            else
            {
                if (__x == __x->_M_parent->_M_left)
                {
                    __x = __x->_M_parent;
                    _Rb_tree_rotate_right(__x, __root);
                }
                __x->_M_parent->_M_color = _S_rb_tree_black;
                __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
                _Rb_tree_rotate_left(__x->_M_parent->_M_parent, __root);
            }
        }
    }
    __root->_M_color = _S_rb_tree_black; // 根节点必为黑
}
```

## 删除太长了,书上没写（逃）