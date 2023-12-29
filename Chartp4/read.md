## Function 语意学

```c++
Point3d Point3d::normalize() const {
    register float mag = magnitude();
    Point3d normal;
    
    normal._x = _x / mag;
    normal._y = _y / mag;
    normal._z = _z / mag;
    return normal;
}
```
C++支持三种类型member function: static、nonstatic和virtual，每一种类型被调用的方式都不相同，其中有两个原因：1）直接存取nonstatic 数据 2）它被声明为const。使得，static member function不可能做到这两点。

原始的”C with Classes“只支持nonstatic member functions。Virtual 函数在20世纪80年代中期被加进来，并且受到许多质疑。Static member functions是最后被引入的一种函数类型，在1987年Useix C++研讨会的厂商研习营中被正式提议加入C++中，并由cfront2.0实现出来。

#### Nonstatic  Memeber Function（非静态成员函数）
C++设计准则之一就是：nonstatic member function至少和一般的nonmember function有相同的效率。也就是，如果在以下两个函数之间做选择：
```c++
float magnitude3d(const Point3d* _this) { ... }
float Point3d::magnitude3d() const { ... }
```
那么选择member function不应该带来什么额外负担，这是因为编译器内部已经将”member函数实体“转换为对等的”nonmember函数实体“
```c++
// magnitude()的一个nonmember 定义：
flaot magnitude3d(const Point3d* _this) {
    return sqrt(_this->_x * _this->_x + 
                _this->_y * _this->_y + 
                _this->_z * _this->_z);
}
```
乍看之下似乎nonmember function比较没有效率，它间接地有参数取用坐标成员，而member function是直接取用坐标成员。然而，它实际上member function被内化为nonmember的形式。

1) 改写函数的signature（指函数原型）
```c++
//1. non-const nonstatic member 转化
Point3d Point3d::magnitude(Point3d* const this) { ... }

//2. const nonstatic member转化
Point3d Point3d::magnitude(const Point3d* const this { ... }
```
2) 对每一个”nonstatic data member“的存取操作改为经由this指针来存取

3) 将member function重新改成一个人外部函数，对函数名称进行*mangling*处理，使它程序中独一无二:
```c++
extern  magnitude_7Point3dFv(register Point3d* const this);

// 于是 obj.magnitude() => 
magnitude_7Point3dFv(&obj);

// 而ptr->magnitude() => 
magnitude_7Point3dFv(ptr);
```

本章的normalize()函数被转化为下面形式：（假设声明有一个Point3d copy construct, 而named returned value(NRV)优化也已经施行）
```c++
void normalize_7Point3dFv(register const Point3d* const this, Point3d& __result) {
    register float mag = this->magnitude();
    
    // default construct
    __result.Point3d::Point3d();
    __result._x = thix->_x / mag;
    __result._y = this->_y / mag;
    __result._z = this->_z / mag;
    return ;
}
```
一个比较有效率的方式是直接构建”normal“的值:
```c++
Point3d Point3d::normal() const {
    register float mag = magnitude();
    return Point3d(_x / mag, _y / mag, _z / mag);
}
```

它会被转化为如下代码：
```c++
void normalize_7Point3dFv(register const Point3d* const this, Point3d& __result) {
    register flaot mag = this->magnitude();
    // __result取代返回值:
    __result.Point3d::Point3d(thix->_x / mag, this->_y / mag, this->_z / mag);
    return ;
}
```
这样可以节省default construct带来的负担

#### 名称的特殊处理(Name Mangling)
一般而言，member的名称前会加上独一无二的class名称，形成独一无二的命名:
```c++
class Bar { public: int ival; ... };

// 其中ival有可能变成这样：
ival__3Bar;
```

为什么编译器要这么做？请考虑这样的派生操作：
```c++
class Foo: public Bar {public: int ival; ... };

// 记住Foo对象内部包含base class和derived class两者：
class Foo {
public:
    int ival__3Bar;
    int ival__3Foo;
};
```
不管你处理哪一个ival，通过"name mangling"，都可以决定清楚的指出来，由于member function可以被重载（overloaded），所有需要更广泛的mangling手法，以提供独一无二的名称。如果把：
```c++
class Point {
public:
    void  x(float newX);
    float x();
};

// 转化为
class Point {
public:
    void  x__5Point(float newX);
    float x__5Point();
};
```
导致两个被重载的函数实体拥有相同的名称。为了让它们独一无二，唯有再加上的它们的参数链表。
如果把参数类型编码也加进去，就一定可以制造出独一无二的经过，使我们两个`x()`函数有良好的转化（**如果你声明`extern C`，就会压抑nonmember function的`managling`效果**）：
```c++
class Point {
public:
    void  x__5PointFf(float newX);
    float x__5PointFv();
};
```
把参数和函数名称编码在一起，编译器于是在不同被编译的模块之间达成了一种有限形式的类型检验。举个例子，如果一个print函数被这样定义：
```c++
void print(const Point3d&) { ... }

// 但意外的被这样声明和调用：
void print(const Point3d);
```
两个实体拥有独一无二的name mangling，那么任何不正确的调用操作在连接时期就因无法决议（resolved）而失败。有时候我们乐观的称此为”确保类型安全的链表行为“，我说“乐观的”是因为它只可以捕获函数的标记（signature，即函数名称 + 参数数目 + 参数类型）的错误，如果是返回类型的声明错误，就没办法检查出来！





