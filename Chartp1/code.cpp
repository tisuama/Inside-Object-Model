#include <iostream>

class A {
public:
	A(int x): x_(x) {}
protected:
	int x_;
};

class B: public A {
public:
	using A::A;
	B();
	void show() {
		std::cout << x_ << " " << y_ << std::endl;
	}
private:
	int y_ {2};
};

int main() {
	B b(1);
	b.show();	
}
