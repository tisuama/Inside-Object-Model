#include <iostream>
#include <cmath>

class A {
public:
	static float mignitude() const {
		return std::sqrt(_x * _x + _y * _y + _z * _z);
	}
	
private:
	int _x, _y, _z;	
};

int main() {
	A a;
}	
