#include <iostream>  
#include <string>  
  
class MyClass {  
public:  
	MyClass(): data(nullptr), sz(0) { 
		std::cout << "default construct " << std::endl;
	}

    MyClass(int sz) : data(new int[sz]), sz(sz) {  
        std::cout << "Constructing MyClass with size " << sz << std::endl;  
    }  
  
    MyClass(const MyClass& other) : data(new int[other.sz]) {  
        std::cout << "Copying MyClass with size " << other.sz << std::endl;  
        for (int i = 0; i < sz; ++i) {  
            data[i] = other.data[i];  
        }  
    }  
  
    ~MyClass() {  
        delete[] data;  
        std::cout << "Destroying MyClass with size " << sz << std::endl;  
    }  
  
    int size() const {  
        return sz;  
    }  
  
private:  
    int* data;  
    int sz;  
};  
  
MyClass createMyClass(int size) {  
    MyClass obj(size);  
    return obj; // NRO optimization can be applied here  
}  
  
int main() {  
	{
		std::cout << "=== start test 1 " << std::endl;
    	MyClass obj = createMyClass(10); // NRO optimization may reduce copying here  
    	std::cout << "obj size: " << obj.size() << std::endl;  
	}

	{
		std::cout << "=== start test 2 " << std::endl;
		// test2
		MyClass test2 = MyClass(2);
	}
    return 0;  
}
