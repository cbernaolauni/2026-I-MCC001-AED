#include <iostream>
#include "macros.h"
#include "lists.h"
// #include "containers/vector.h"

using namespace std;

// g++ -std=c++2b main.cpp vector.cpp macros.cpp -o main
// 2011: C++11 => C++14 => C++17 => C++20 => C++23 => C++26

size_t size1() { return 0; }
unsigned char size2() { return 0; }
int main() {
    /*cout << "HelloX World MCS-AED 2026-I" << endl;
    unsigned long i1 = size1()-1;
             long i2 = size1()-1;
    cout << "-1 en un size_t     : " << i1 << endl;
    cout << "-1 en un signed long: " << i2 << endl;

    unsigned char i3 = size2()-1;
             char i4 = size2()-1;
    cout << "-1 en un char       : " << (int)i3 << endl;
    cout << "-1 en un char       : " << (int)i4 << endl;

    int i = 5000;
    unsigned char i5 = i; // 136
             char i6 = i; // -120
    cout << "i5: " << (int)i5 << endl;
    cout << "i6: " << (int)i6 << endl;*/

    // DemoMacros();
    //DemoVector();
    // DemoConcurrentVector();
    // ListsDemo();
    HeapDemo();
    
    return 0;
}
