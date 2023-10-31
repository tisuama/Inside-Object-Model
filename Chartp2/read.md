## 2.1 构造函数语意学
ARM => C++ Annotated Reference Manual

C++ Standard在Section 12.1说
>	对于class X，如果没有任何user-declared constructor，那么会有一个default constructor被暗中(implicitly)声明出来，它将是一个trivial(没啥用的)constructor。


```c++
class Foo {
public:
	int val;
	Foo *next;
};

void foo_bar() {
	Foo bar;  // 程序要求bar的data members都初始化为0
	if (bar.val || bar.next) {
		// do something
	}
}
```

什么时候才会合成一个default constructor呢？当编译器需要它的时候！此外合成出来的constructor只执行编译器所需要的行动。也就是说，即使有需要为class Foo合成一个default constructor，那个constructor也不会将两个data members 初始化为0。

### Nontrivaial default constructor

Nontrivaial default constructor在ARM的术语中就是编译器所需要的哪种，必要的化会由编译器合成。

a) 带有default constructor的member class object
如果一个class没有任何constructor，但它有一个member object，而后者有default constructor，那么这个implicit default constructor即是"nontrivial"。

如果有多个class member objects都要求constructor初始化操作，将如何呢？C++语言要求以member objects在class中的声明次序来调用各个constructors，这一点由编译器完成，它为每一个constructor安插程序代码，以“members声明次序”调用每一个member所关联的default constructors。



b) 带有default constructor的base class
类似的道理，如果没有任何一个construct 的class派生自一个“带有default constructor“的base class，那么这个derived class的default constructor会被视为nontrivial。


c）带有virtual function的class
class声明（或继承）一个virtual function

![vptr](./vptr.png)

d) 带有virtual base class的class
![bptr](./bptr.png)




### copy constructor的构建操作


三种情况：
```c++
class X { ... };

X x;

X xx = x; // case 1：以一个object的内容作为另一个class object的初值


extern void foo(X x);

X xx;
foo(xx); // case 2：参数


X foo_bar() {
	X xx;
	return xx; // case 3：返回值
}

```

一个良好的编译器可以为大部分class object产生bitwise copies，因为它们有bitwise coy semantics语义。

向default  constructor一样，如果class没有声明一个copy constructor，就会隐含的声明或隐含的定义出现。和以前一样，copy constructor也分为trival和nontrival两种，区别是class是否展现出所谓的'bitwise copy constructor'。


## 2.2 bitwise copy semantics（位逐次拷贝）

```c++
class Word { // bitwise copy semantics（位逐次拷贝）语义
public:
	Word(const char*);
	~Word( {delete [] str;}
private:
	int cnt;
	char* str;
};

class Word {
public:
	Word(const String&);
	~Word();

private:
	int cnt;
	String str;
};

其中String声明一个explicit copy constructor：
class String {
public:
	String(const char*);
	String(const String&);
	~String();
};
```
## 不要BitWise Coy Semantics

什么时候不展现"bitwise copy semantics呢？存在四种情况：

1. class含有member object，后者声明一个copy constructor（无论是被明确声明或者被合成)
2. class继承一个base class，后者存在copy constructor（无论是被明确声明或者被合成）
3. class声明了一个或多个virtual functions
4. class派生自一个继承链，其中有一个或多个virtual base class
其中1、2两种情况中，编译器必须将member或者base class的'copy constructor'调用操作安插到被合成的conpy constructor中。


### 重新设定virtual table指针（情况3）

编译期间程序扩张操作：
1. 增加一个virtual function table(vtbl)，内含每一个有作用的virtual function地址
2. 将一个virtual function table的指针(vptr)安插在class object内部

### 处理virtual base class subobject（情况4）

virtual base class的存在需要特殊处理。一个class object如果以另一个object作为初始值，而后者有一个virtual base class subobject，那么也会使'bitwise copy emantics'失效。


那么memberwise初始化了，一个virtual base class的存在会使bitwise copy semantics无效，其次，问题并不发生于“一个class object以另一个同类object作为初始值时”， 而是发生于“一个class object以其derived class的某个object作为初值时。

```c++
class RedPanda: public Raccoon {
public:
	RedPanda() {}
	RedPanda(int val) {}
private:
	// 必要数据
};

Raccoon rocky;	
Raccoon little_critter = rocky; // bitwise copy足够

RedPanda little_red;
Raccoon little_critter = little_red; // X

// 这种情况下，为了完成正确的little_critter初值设定，编译器必须合成一个copy constructor，安插一些代码以设定virtual base class  pointer/offset的初值，以每一个member执行必要的memberwise初始化操作。
```
>	
在C++中，virtual base class pointer和virtual base class offset是与面向对象编程中的多态性（polymorphism）相关的概念。

虚拟基类指针（virtual base class pointer）是指在派生类对象中用来存储基类对象的指针。在C++中，如果一个基类被声明为虚基类（使用virtual关键字），则在每个派生类对象中都会包含一个指向该基类对象的指针。通过这个指针，可以在运行时动态地访问虚基类对象。

虚拟基类偏移量（virtual base class offset）是指在派生类对象中虚基类对象相对于派生类对象起始位置的偏移量。这个偏移量是在编译时计算并存储在派生类对象中的。当通过虚基类指针访问虚基类对象时，可以使用这个偏移量来确定虚基类对象在派生类对象中的位置。
虚拟基类指针和偏移量是在实现多态性时使用的机制，它们允许在运行时根据对象的实际类型来访问基类对象。这样可以在不同的派生类对象中使用相同的基类接口，而实现各自不同的行为。这种灵活性是C++面向对象编程的重要特点之一。

### bitwise copy semantics总结
已经看到四种情况，class不在保持'bitwise copy semantics'， 而且default copy construct如果未被声明，会被视为是notrivial。在这四种情况下，如果缺乏一个已声明的copy construct，编译器为了正确处理”以一个class object作为另一个class object的初值“，必须合成出一个copy constructor。


## 2.3 程序转化语义学
```c++
X foo() {
	X xx;
	// ...
	return xx;
}
```
1. 每次foo()被调用，就传回xx的值？？
2. 如果Class X定义了一个copy construct，那么当foo()被调用时，保证该copy construct被调用？？

### 明确的初始化操作
```c++
X x0
void foo_bar()  {
	X x1(x0);
	X x2 = x0;
	X x3 = X(x0);
}

转化为
void foo_bar() {
	X x1, x2, x3;

	// 编译器安插X的copy construct调用操作
	x1.X::X(x0);
	x2.X::X(x0);
	x3.X::X(x0);
}
```
#### 参数初始化
C++ standard中说吗，把一个class object当做参数传递给一个函数，相当于一下形式的初始化操作
```c++
void foo(X x0)

// 调用
X xx
foo(xx);

// 编译器实现为：
X __temp0;
// copy constructor调用
__temp0.X::X(xx);

// 改写函数调用操作，以使用临时对象
foo(__temp0);

// 此处声明也要修改，否则会调用拷贝构造
void foo(X& x0);
```

#### 返回值的初始化
已知这个函数含义：
```
X bar() {
	X xx;
	return xx;
}
```
bar()返回值如何从局部对象xx中拷贝过来？
1. 首先加上一个额外参数，类型时class object的reference，这个参数将会放置被”拷贝构建“而得到的返回值。
2. 在return指令之前安插一个copy construct调用操作。

这里可以有两种优化方式：
```c++
void bar(X& __result) {
	X xx;
	xx.X::X();
	// 处理xx...
	
	// 调用拷贝构造函数
	__result.X::X(xx);	
	return ;
}
```

#### 编译器层面优化
在bar()这样函数中，所有return指令传回相同的具名数值，因此编译器可能自己做优化，方法是以result参数取代name return value。例如下面bar()定义:
```c++
X bar() {
	X xx;
	return xx;
}

// 编译器把其中xx以_result取代：
void bar(X& __result) {
	// default construct被调用
	__result.X::X();
	// 直接处理__result...
	return ;
}
```
这样的编译器优化操作，被称为`Named Return Value(NRV)`优化，NRV优化如今被视为标准C++编译器义不容辞的优化操作。为了对效率有所改善，请你想下以下代码：

`NRV`优化本质上是优化掉拷贝构造函数，并不是使用了NRV优化选项就一定能用NRV优化。
```c++
class test {
friend test foo(double) ;
public
	test() {
		memset(array, 0, 100 * sizeof(double));
	}
private:
	double array[100];
};

// 考虑一下函数，他产生、修改并传回一个test class object:
test foo(double val) {
	test local;
	local.array[0] = val;
	local.array[99] = val;
	return local;
}
// double数组影响NRO优化需要相应的拷贝构造函数

int main() {
	for (int i = 0; i < 10000000; i++) {
		test t = foo(double(i));
	}
	return 0;
}
```
#### 总结
一般而言，蛮对”以一个class object作为另一个class object的初值的情形，语言允许编译器有大量的自由发挥空间，其利益当然是导致机器码产生时有明显的效率提升，缺点则是不能够安全的规划你的copy construct的副作用，必须视其执行而定。

### Copy Constructory要还是不要？
```c++
class Point3d {
public:
	Point3d(float x, float y, float z);
private:
	float _x, _y, _z;
};
```

Point3d的default copy constructor被视为trivial，它既没有任何member（或base) class object带有copy constructor，也没有任何virtual base class或virtual function。所以，默认情况下，一个Point3d对象的`memberwise`初始化操作会导致bitwise coy，这样效率很高，安全性了？

答案是"yes"，`biewise copy`既不会导致memory leak,也不会产生address aliasing，因此它既快速又安全。

```c++
Point3d operator+(const Point3d&, const Point3d&);
Point3d operator-(const Point3d&, const Point3d&);
Point3d operator*(const Point3d&, int);

// 所有下列类型函数都能符合NRV template
{
    Point3d result;
    return result;
}

// 实现copy construct最简单方法：
Point3d::Point3d(const Point3d& rhs) {
    _x = rhs._x;
    _y = rhs._y;
    _z = rhs._z;
}

// 使用memcpy实现更有效率:
Point3d::Point3d(const Point3d& rhs) {
    memcpy(this, &rhs, sizeof(Point3d));
}

/*
然而，无论是`memcpy`或者`memset`都只在class不含任何由编译器产生内部member时才能有效运行// ，如果Point3d声明一个或一个以上virtual functions，或内含一个virtual base class，那么使用上述函数会导致那些“被编译器产生的内部member”的初值被改写，例如：
*/
class Shape {
public:
    // 错误：会改变内部vptr
    Shape(): { memset(this, 0, sizeof(Shape)) };
    virtual ~Shape();
};

// 编译器被construct扩张的内容看起来是：
Shape::Shape() {{
    __vptr__Shape = _vtbl__Shape;
    // memset会清空vptr
    memset(this, 0, sizeof(Shape));
}
```

copy constructor的应用，迫使编译器多多少少对你的程序代码做部分转化，尤其当一个函数以传值的方式返回一个class object，而该class有一个copy construct（无论是明确定义还是合成的）时，这将导致深奥的程序转化-无论是在函数定义还是使用上。此外编译器也将copy construct的调用操作优化，以一个额外的第一参数取代NRV。


#### 成员们的初始化队伍
首先要弄明白何时使用`initialization list`才有意义。

```c++
class Word {
    String _name;
    int    _cnt;
public:
    Word() {
        _name = 0;
        _cnt = 0;
    }
};

// 可能展开成一下形式：
Word::Word {
    // 调用Stirng 的default construct
    _name.String::String();

    // 产生临时对象
    String temp = String(0);

    // memberwise的拷贝_name
    _name.String::operator=(temp);

    // 摧毁临时对象
    temp.String::~String();

    _cnt = 0;
}

// 明显可能的优化方式：
Word::Word:_name(0) {
    _cnt = 0;
}

// 可被展开为
Word::Word {
    _name.String::String(0);
    _cnt = 0;
}

```

我们不禁要提出一个合理的问题：member initialization list中到底会发生什么事情？编译器会一一操作initialization list，以适当的次序在construct之内安插初始化操作，并且在任何explicit user code之前。

另一个常见的问题是，是否你能像下面这样，调用一个`member function`以设定一个member的初值：
```c++
// X::xfoo()被调用，这样好吗？
X::X(int val)
    : i(xfoo(val)), j(val)
{}
```
