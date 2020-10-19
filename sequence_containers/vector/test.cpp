#include "my_vector.h"
#include <iostream>

using namespace std;

int main()
{
    MOG::vector<int> a;
    a.push_back(1);
    cout << a.size() << "\n";
    cout << a.capacity() << "\n";
    a.push_back(2);
    cout << a.size() << "\n";
    cout << a.capacity() << "\n";

    a.push_back(3);
    cout << a.size() << "\n";
    cout << a.capacity() << "\n";

    a.push_back(4);
    a.push_back(5);
    a.push_back(6);

    a.insert(a.begin() + 3, 6);

    for (int i : a)
    {
        cout << i;
    }
    cout << "\n";
    a.erase(a.begin() + 3);

    for (int i : a)
    {
        cout << i;
    }
    cout << "\n";
    a.clear();
    cout << a.size() << "\n";
    cout << a.capacity() << "\n";
}