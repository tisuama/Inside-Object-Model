#include <iostream>
#include <cmath>

class A1 {
	public:
		virtual void show() { std::cout << "A1" << std::endl; }
		void show_this() { std::cout << "A1 " << this << std::endl; }
};

class B1 {
	public:
		virtual void show() { std::cout << "B1" << std::endl; }
};

class C1: public A1, public B1 {
	public:
		void show() { std::cout << "C1" << std::endl; }
};

class A2: public virtual A1 {
	public:
		void show_this() { std::cout << "A2 " << this << std::endl; }
};

class Point {
public:
	virtual ~Point() {};
	void x() { std::cout << "x" << std::endl; }
	void y() { std::cout << "y" << std::endl; }
	virtual void z() { std::cout << "virtual z" << std::endl; }
	virtual void p() { std::cout << "virtual p" << std::endl; }

public:
	int v;
};

int main() {
	C1 c;
	// 编译错误
	c.show();
	std::cout << sizeof(c) << std::endl;
	A2 a2;
	std::cout << sizeof(a2) << std::endl;
	a2.show_this();
	A1* a1 = &a2;
	a1->show_this();

	int Point::* pv = &Point::v;
	void (Point::*pmx)() = &Point::x;
	void (Point::*pmy)() = &Point::y;
	void (Point::*pmz)() = &Point::z;
	void (Point::*pmp)() = &Point::p;
	Point* ptr = new Point;
	(ptr->*pmz)();	
	(ptr->*pmp)();
	(ptr->*pmx)();

	printf("v: %p\n", pv);
	printf("x: %p\n", pmx);
	printf("y: %p\n", pmy);
	printf("z: %p\n", pmz);
	printf("p: %p\n", pmp);
}	
