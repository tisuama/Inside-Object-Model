#include <iostream>

class A1 {
public:
    // ...
    int val;
    char bit1;
};

class A2: public A1 {
public: 
    // ...
    char bit2;
};

class A3: public A2 {
public: 
    // ...
    char bit3;
};

int main() {
	std::cout << sizeof(A1) << std::endl;
	std::cout << sizeof(A2) << " " << sizeof(A3) << std::endl;
	std::cout << a2.bit2 << std::endl;
}
