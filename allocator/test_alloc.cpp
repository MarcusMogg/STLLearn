#include <algorithm>
#include <iostream>
#include <vector>

#include "my_alloc.h"
using namespace std;

int main()
{
    MOG::simple_alloc<int, MOG::default_alloc> test;
    int *a = test.allocate(10);
    for (int i = 0; i < 10; ++i)
    {
        a[i] = i;
    }
    for (int i = 0; i < 10; ++i)
    {
        cout << a[i];
    }
    test.deallocate(a, 10);
    a = test.allocate(5);
    for (int i = 0; i < 5; ++i)
    {
        a[i] = i + 10;
    }
    for (int i = 0; i < 5; ++i)
    {
        cout << a[i];
    }
    test.deallocate(a, 5);
    a = static_cast<int *>(MOG::default_alloc::allocate(1 * sizeof(int)));
    a[0] = 1;
    cout << a[0];
    a = static_cast<int *>(
        MOG::default_alloc::reallocate(a, 1 * sizeof(int), 10 * sizeof(int)));
    for (int i = 1; i < 10; ++i)
    {
        a[i] = i + 10;
        cout << a[i];
    }
    a = static_cast<int *>(
        MOG::default_alloc::reallocate(a, 10 * sizeof(int), 100 * sizeof(int)));
    for (int i = 10; i < 100; ++i)
    {
        a[i] = i + 100;
        cout << a[i];
    }
    a = static_cast<int *>(
        MOG::default_alloc::reallocate(a, 100 * sizeof(int), 1000 * sizeof(int)));
    for (int i = 100; i < 1000; ++i)
    {
        a[i] = i + 1000;
        cout << a[i];
    }
    for (int i = 0; i < 1000; ++i)
    {
        cout << a[i] << "\n";
    }
    MOG::default_alloc::deallocate(a, 1000 * sizeof(int));
    return 0;
}