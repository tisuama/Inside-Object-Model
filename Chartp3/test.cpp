#include <iostream>
#include <stdio.h>

struct Point3d {
public:
	virtual ~Point3d();

	static Point3d origin;
	float x, y, z;
};

int main() {
	printf("%p", &Point3d::x);
}
