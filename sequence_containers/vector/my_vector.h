#pragma once
#ifndef _MY_VECTOR_H_
#define _MY_VECTOR_H_
#include "../../allocator/my_alloc.h"
#include "../../allocator/my_construct.h"

#include <memory>
#include <algorithm>

namespace MOG
{
template <class T, class Alloc = default_alloc>
class vector
{
public:
    typedef T value_type;
    typedef value_type *pointer;
    typedef value_type *iterator; // vector的迭代器就是普通指针
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

protected:
    typedef simple_alloc<T, Alloc> data_allocator;

    iterator start;          // 目前使用空间的起始位置
    iterator finish;         // 目前使用空间的结束位置
    iterator end_of_storage; // 目前可用空间的结束位置 要区分可用和使用空间

    iterator allocate_and_fill(size_type n, const_reference x)
    {
        iterator result = data_allocator::allocate(n);
        std::uninitialized_fill_n(result, n, x);
        return result;
    }

    void insert_aux(iterator position, const_reference x);
    void deallocate()
    {
        if (start) // 判断null
            data_allocator::deallocate(start, end_of_storage - start);
    }
    void fill_initialize(size_type n, const_reference x)
    {
        start = allocate_and_fill(n, x);
        finish = start + n;
        end_of_storage = finish;
    }

public:
    iterator begin() { return start; }
    iterator end() { return finish; }
    size_type size() const { return size_type(finish - start); }
    size_type capacity() const
    {
        return size_type(end_of_storage - start);
    }
    bool empty() const { return begin() == end(); }
    reference operator[](size_type n) { return *(begin() + n); }

    vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}
    vector(size_type n, const_reference x) { fill_initialize(n, x); }

    explicit vector(size_type n) { fill_initialize(n, T()); } // explicit会禁止隐式转换, 防止 vector a = 1这样的出现

    ~vector()
    {
        destroy(start, finish);
        deallocate();
    }

    reference front() { return *begin(); }
    reference back() { return *(end() - 1); }

    void push_back(const_reference x)
    {
        if (end() != end_of_storage)
        {
            construct(end(), x);
            ++finish;
        }
        else
        {
            insert_aux(end(), x);
        }
    }

    void pop_back()
    {
        --finish;
        destroy(finish);
    }

    // 只析构，没有释放空间
    iterator erase(iterator position)
    {
        if (position + 1 != end())
        {
            std::copy(position + 1, finish, position);
        }
        --finish;
        destroy(finish);
        return position;
    }
    iterator erase(iterator first, iterator last)
    {
        iterator nf = std::copy(last, finish, first); // copy返回值是目标范围中最后复制元素的下个元素的输出迭代器。
        destroy(nf, finish);
        finish -= (last - first);
        return first;
    }
    void insert(iterator position, const_reference x)
    {
        insert(position, 1, x);
    }
    void insert(iterator position, size_type n, const_reference x);

    void resize(size_type new_size, const_reference x)
    {
        if (new_size <= size())
        {
            erase(begin() + new_size, end());
        }
        else
        {
            insert(end(), new_size - size(), x);
        }
    }
    void resize(size_type new_size) { resize(new_size, value_type()); }
    void clear() { erase(begin(), end()); }
};

template <class T, class Alloc>
void vector<T, Alloc>::insert_aux(iterator position, const_reference x)
{
    if (finish != end_of_storage)
    {
        construct(finish, *(finish - 1));
        ++finish;
        std::copy_backward(position, finish - 2, finish - 1);
        value_type x_copy = x;
        *position = x_copy;
    }
    else
    {
        const size_type old_size = size();
        const size_type new_size = old_size == 0 ? 1 : old_size * 2;

        iterator new_start = data_allocator::allocate(new_size);
        iterator new_finish = new_start;
        try
        {
            new_finish = std::uninitialized_copy(start, position, new_start);
            construct(new_finish, x);
            ++new_finish;
            new_finish = std::uninitialized_copy(position, finish, new_finish);
        }
        catch (...)
        {
            destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, new_size);
            throw;
        }
        destroy(begin(), end());
        deallocate();

        start = new_start;
        finish = new_finish;
        end_of_storage = start + new_size;
    }
}

template <class T, class Alloc>
void vector<T, Alloc>::insert(iterator position, size_type n, const_reference x)
{
    if (n != 0)
    {
        if (n <= size_type(end_of_storage - finish))
        {
            value_type x_copy = x; // 为啥？ 考虑拷贝构造函数和拷贝赋值函数可能不同吗？
            // oc,参数是引用类型的，有可能在复制过程中发生了改变，比如insert(a.begin()+2,5,a[3]);
            const size_type elems_after = finish - position;
            iterator old_finish = finish;
            if (elems_after > n)
            {
                /*copy 是依次调用重载的运算符=,
                      uninitialized_copy是依次调用拷贝构造函数
                      如果目标区间是未初始化的，应该用uninitialized_copy， 否则用copy*/
                std::uninitialized_copy(finish - n, finish, finish);
                finish += n;
                std::copy_backward(position, old_finish - n, old_finish);
                std::fill(position, position + n, x_copy);
            }
            else
            {
                std::uninitialized_fill_n(finish, n - elems_after, x_copy);
                finish += n - elems_after;
                std::uninitialized_copy(position, old_finish, finish);
                finish += elems_after;
                std::fill(position, old_finish, x_copy);
            }
        }
        else
        {
            const size_type old_size = size();
            const size_type new_size = old_size + std::max(old_size, n);

            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try
            {
                new_finish = std::uninitialized_copy(start, position, new_start);
                new_finish = std::uninitialized_fill_n(new_finish, n, x);
                new_finish = std::uninitialized_copy(position, finish, new_finish);
            }
            catch (...)
            {
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }
            destroy(begin(), end());
            deallocate();

            start = new_start;
            finish = new_finish;
            end_of_storage = start + new_size;
        }
    }
}
} // namespace MOG

#endif
