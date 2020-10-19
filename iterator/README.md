## 迭代器设计思维

书上有一句话说的很好，stl的中心思想是将容器和算法分开，彼此独立设计，最后再用一种胶着剂将其撮合起来。

迭代器就是这样的胶着剂

比如find(first,last,val)，只要提供统一接口的迭代器，find就可以适应各种容器

## 迭代器是一种smart pointer

迭代器的行为是类似于指针的

迭代器最重要的工作就是对`operator *`和`operator ->`进行重载

为什么每一种 STL 容器都提供有专属迭代器？ 主要是暴露太多细节，所以把迭代器的开发工作交给容器去完成，这样所有实现细节可以得到封装，不被使用者看到。

## 迭代器相应类型(associated types)

有一个问题是，算法中运用迭代器的时候，有时候需要用到其相应类型。比如所指元素的类型。因为c++不支持typeof，所以无法直接获取、

可以利用 function template 的参数推导机制，推导出参数的类型(无法推导出函数返回值类型)。

```cpp

template<class I, class T>
void func_impl(I iter, T t)
{
    // 这样我们就可以声明对应类型的变量
    T tmp;
}

template<class I>
void func(I iter)
{
    func_impl(iter, *iter);
}

int main()
{
    int i;
    func(&i);
}
```

当然上面是c++ 11之前的做法了，c++11 提供了 decltype ，它的作用是选择并返回操作数的数据类型。只计算类型不计算表达式的值

```cpp
template<class I>
void func(I iter)
{
    // decltype(*iter)的类型是一个int&,remove_reference去除引用
    typedef std::remove_reference<decltype(*iter)>  T;
    T tmp;
}

int main()
{
    int i;
    func(&i);
}
```

但是上面的方法还是不行，不能用于返回类型的声明，所以需要更通用的解决方案

## Traits 编程技术

traits 意为 “特性”，扮演 “特性萃取机” 角色，萃取各个迭代器的特性(相应类型)。

迭代器相应类型有五种：
  - value type 迭代器所指对象类型(右值类型)
  - difference type 两个迭代器之间距离
  - pointer 指针(左值)，operator ->的返回类型
  - reference 引用(左值)，operator *的返回类型
  - iterator category 迭代器类型？还挺复杂的

如何获取这些类型呢？

一个想法是在实现类里面使用typedef，但是这样不支持原生指针。stl应该接受原生指针。所以应该用更广泛的做法

template partial specialization 模板偏特化：针对 template 参数更进一步的条件限制所设计出来的一个特化版本，本身仍然是 template。

value_type 的对应实现

```cpp
template<class I>
struct my_iterator_trait
{
    // 在模板的声明或定义内，typename 可用于声明某个待决的有限定名是类型。
    // 意思是，本来不知道I::value_type是个成员变量还是啥，typename声明了它是一个类型
    typedef typename I::value_type value_type;
};

template<class T>
struct my_iterator_trait<T*> // 偏特化版本，对应原生指针
{
    typedef T value_type;
};

template<class T>
struct my_iterator_trait<const T*> // 偏特化版本，对应const原生指针，上一个会返回const T
{
    typedef T value_type;
};

template<class I>
typename my_iterator_trait<I>::value_type func(I iter)
{
    return *iter;
}

int main()
{
    int i;
    auto b = func(&i);
}
```

任何迭代器都应该支持以上五种相应类型

为了简化操作，stl提供了一个iterator class（统一的接口）

```cpp
template<class _Category,
	class _Ty,
	class _Diff = ptrdiff_t,
	class _Pointer = _Ty *,
	class _Reference = _Ty&>
	struct iterator
	{	// base type for iterator classes
	using iterator_category = _Category;
	using value_type = _Ty;
	using difference_type = _Diff;
	using pointer = _Pointer;
	using reference = _Reference;
	};
```

可以（最好）继承这个class来简化定义

```
template<class T>
struct test_iterator:public iterator<forward_iterator_tag,T>
{
};
```