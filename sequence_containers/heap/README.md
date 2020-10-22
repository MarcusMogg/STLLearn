## heap 

heap 可以理解为一个完全二叉树（即除了最底层其他都是满的，最底层尽量靠左），且满足一个节点的值是以该节点为根节点的最大（小）值

stl里面默认是最大堆

### 存储方式

heap默认是存储到vector里（因为需要支持扩容）

以中序遍历的方式将heap的节点存到vector里，那么一个节点`i`的左孩子是 `2i + 1`,右孩子是`2i+2`，父节点是`(i-1)/2`

注意这里i是序号，不是存的值

例子：

```
[0,1,2,3,4...]
```
相当于

```
    0
   / \
  1   2
 / \
3   4 ...
```

### push

push一个值会先将该值存储到vector末尾（end位置）,然后将该值和父节点比较，如果大于父节点的值，则将该节点与父节点交换，重复上述过程直到到达根节点或者小于父节点的值。

```cpp
    void push(const T &value)
    {
        data.push_back(value);
        size_t index = size, parent = (index - 1) / 2;
        size++;
        while (index > 0 && value > data[parent])
        {
            data[index] = data[parent];
            index = parent;
            parent = (index - 1) / 2;
        }
        data[index] = value;
    }
```

### pop

```cpp
    void pop()
    {
        sz--;
        swap(data[0], data[sz]); // 先将首尾交换
        size_t index = 0, rch = 0;
        T value = data[0];
        rch = 2 * (index + 1); // 右孩子
        while (rch < sz)
        {
            if (data[rch] < data[rch - 1])
                rch--; // 选取左右孩子中的最大值

            data[index] = data[rch]; // 将孩子中的最大值上移到父节点

            index = rch;
            rch = 2 * (index + 1);
        }
        if (rch == sz) // 当前节点只有左孩子没有右孩子
        {
            rch--;
            data[index] = data[rch];
            index = rch;
        }

        // 书上说可以用 data[index] = value 替换，应该是错误的
        push_heap(data.begin(), index, 0, value);
    }
```

最后为什么会有一个push_heap呢？考虑这样一个堆

```
        10
        / \
       9   5
      / \  / \
     1   2 3  4
```

调用上面函数之后，在push_heap之前结果为：
```
        9
        / \
       2   5
      / \  / 
     1   ? 3  
```
value 为4，填到?处是不满足性质的

书上说可以用 data[index] = value 替换，应该是错误的。不过据说在新版书上已经改了，旧版书落泪

### heap sort

pop里那个函数每次会吧最大值移动到数组尾部，调用n次就得到一个递增序列，复杂度是nlogn

### make heap

将一个乱序的序列调整为heap

从`(sz - 2) / 2`（完全二叉树的性质，`(sz - 2) / 2`是最后一个有孩子的节点）开始调用pop里那个函数里的调整部分（不首尾交换，stl里是分开的）。往前一直调用到根节点

### heap没有迭代器

## priority_queue

priority_queue 底层就是heap，没啥说的