#include <vector>
#ifndef _MY_HEAP_H_
#define _MY_HEAP_H_
namespace MOG
{
template <class T>
class my_heap
{
private:
    std::vector<T> data;
    size_t sz;

    // 调用此函数时，新值已在vector里
    template <class RandomAccessIterator, class Distance, class T>
    void push_heap(RandomAccessIterator first, Distance index, Distance topIndex, T value)
    {
        Distance parent = (index - 1) / 2;
        while (index > topIndex && value > data[parent])
        {
            data[index] = data[parent];
            index = parent;
            parent = (index - 1) / 2;
        }
        data[index] = value;
    }

public:
    void push(const T &value)
    {
        data.push_back(value);
        push_heap(data.begin(), sz, 0, value);
        sz++;
    }

    T pop()
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

        value = data[sz];
        data.pop_back();
        return value;
    }
};
} // namespace MOG
#endif