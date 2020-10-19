#include "my_allocator.h"
#include <vector>
#include <iostream>

using namespace std;

int main()
{
    int a[5] = {0, 1, 2, 3, 4};
    vector<int, MOG::allocator<int>> v(a, a + 5);
    for (size_t i = 0; i < 5; i++)
    {
        cout << v[i];
    }
}