## Data 语意学
一个空的class如：
```c++
// sizeof X == 1
class X {};
```
事实上，X并不是空的，它有一个隐晦的1 byte，那是编译器安插进去的一个char，这使得这个class的两个objects得以在内存配置独一无二的内存地址：
```c++
X a, b;
if (&a == &b) {
    cerr < "yipes!" << endl;
}
```

但是Y和Z的sizeof结果却是：
```c++
// sizeof Y == sizeof Z == 8
class Y: public virtual X {};
class Z: public virtual X {};
class A: public Y, public Z {};
```
事实上，Y、Z的大小受到三个因素影响：
1. 语言本身所造成的额外负担（overhead)
2. 编译器对特殊情况所提供的优化处理。
3. Alignment限制，class Y和Z大小截止目前为5 byte（32位？）大部分机器上会受到alignment限制

那么你期望A的大小是什么？
如果我们忘记Y、Z都是“虚拟派生”自class X，我们可能回答16。毕竟Y和Z的大小都是8。然而我们对class A施以sizeof 运算符，得到的答案竟然是12。

记住一个virtual base class subject只会在derived class中存在一分实体，不管它在继承体系中出现了多少次，class A的大小由以下几点决定：
1. 被大家贡献的唯一一个class X实体，大小1 byte
2. Base class Y的大小，减去“因 virtual base class X而配置”的大小，结果是4 bytes（说的是没有对sizeof(X) = 1, sizeof(Y) = 1 + 4 = 5(alignment = 8)的情况）
3. class A自己的大小: 0 bytes
4. class A的alignment，调整前是1 + 4 + 4 = 9, alignment = 8

如果我们考虑那种“特别对empty virtual base class做了处理”的编译器，一如前所述，class X实体的那1 byte会被拿掉，于是额外的3 bytes填补也不必了，因此class A的大小是 8 bytes。

### Data Member的绑定
```c++
extern float x;
class Point3d {
public:
    Point3d(float, float, float);
    float X() const { return x; }
    void X(float new_x) const { x = new_x };
private:
    float x, y, z;
};

```
Point3d::X()到底会传回class内部那个x，还是外部的那个x？
C++ Standard以"member scope resolution rules"规定。效果是：

```c++
extern int x;
class Point3d {
    float X() const { return x; };
};

// 事实上，分析在这里进行。

```
对member function本身的分析，会直到整个class声明都出现了才开始。

然而，反对对于member function的argument list并不为真，Argument list中的名称还是会在它们第一次遭遇时被释放的决议（resolve)完成，因此在extern 和 nested type names之间的非直觉绑定操作还会发生。

```c++
typedef int length;
class Point3d {
public:
    void mumble (length val) { _val = val; }
    length mumble() { return _val; }
private:
    // legnth必须在"本class对它第一参考操作”之前被看见
    // 这样的声明将使之前的参考操作不合法
    typedef float length;
    length _val;
};
```
上述情况任然需要某种防御性程序风格：请使用把"nested type"声明放在class起始处。

### Data Memeber 的布局
已知下面一组data members:
```c++
class Point3d {
public:
    // ...
private:
    float x;
    static List<Point3d*> *freeList;
    float y;
    static const int chunkSize = 250;
    float z;
};

```

static data member存放在程序的data segment 中，和个别的class object无关。

C++ Standard要求，在同一个access section中，member的排列只需符合“较晚出现的members在class object中有较高的地址”这一条件极客。也就是，各个member并不一定要连续排列。

编译器还会合成一些内部使用的data members，以支持整个对象模型，vptr就是这样的东西。vptr会放在什么位置呢？传统上它被放在所有明确声明的members最后。不过当前也有一些编译器把vptr放在class object最前端。

### Data Member的存取

#### Static Data Members
Static data members，按照其字面意义，被编译器提出于class之外，视为一个global变量。
每一个static data member只有一个实体，存放在程序的data segment之中，每次程序参阅（取用）static member就会被内部转化为对该唯一extern实体的直接参考操作：
```c++
// origin.chunkSize = 250
Point3d::chunkSize == 250；

// pt->chunkSize = 250;
Point3d::chunkSize = 250;
```

#### Nonstatic Data Members
Nonstatic data members直接存放在每一个class object之中，除非经由明确或暗喻的class object，没有办法直接存取它们。只要程序员在一个member function中直接处理一个nonstatic data member，所谓的"implicit class object"就会发生。
```c++
Point3d
Point3d::translate(const Point3d& pt) {
    x += pt.x;
    y += pt.y;
    z += pt.z;
}
// 实际上被转化为
Point3d
Point3d::translate(Point3d* const this, const Point3d &pt) {
    this->x += pt.x;
    this->y += pt.y;
    this->z += pt.z;
}
// 对一个nonstatic data member进行存取操作，编译器需要把class object的起始地址加上data member的偏移量（offset），例如，
```c++
origin._y = 0.0;
// &origin._y的地址为
&origin + (&Point3d::_y - 1);


// 下面两种情况的差异：
origin.x = 0.0;
pt->x = 0.0;
```
从origin存取和从ptr存取的差异？当Point3d是一个derived class，而在继承结构中有一个virtual base clas，并且存取的member是一个从该virtual base class继承而来的member时，就会有重大差异。

因为我们不能够说pt必然指向哪一种class type（因此编译器时不知道member真正的offet位置），所以这个存取操作必须延迟到执行期间，经由一个外的间接引导，才能够解决。而origin就没有这个问题，其类型无疑是Point3d class，即使继承自virtual base class，member的offset位置在编译期间就能够确定。

#### 继承与Data Member

依次讨论：单一继承且不含virtual funtion、单一继承含virtual function、多重继承、虚拟继承四种情况。

#### a)只要继承不要多态
```c++
class A1 {
public:
    // ...
private:
    int val;
    char bit1;
};

class A2: public A1 {
public: 
    // ...
private:
    char bit2;
};

class A3: public A2 {
public: 
    // ...
private:
    char bit3;
};

// sizeof(A1) = 8
// sizeof(A2) = 12
// sizeof(A3) = 16
```
把两个原本独立不相干的class凑成一对"type/subtype"，并带有继承关系，会有什么易犯的错误呢？
经验不足的人可能会重复设计一些相同操作的函数。
第二个易犯的错误是，把一个class分解成两层或更多层，有可能为了”表现class体系之抽象化“而膨胀所需空间。C++语言保证：”出现derived class中的base class subobject有其完整原样性“

#### b)加上多态

如果我们要处理一个坐标点，而不在乎它是什么Point2d或者Point3d实例，我们需要提供一个virtual function接口。

在C++编译器领域有一个讨论题目：把vptr放置在class object的哪里会更好？

把vptr放在class object的尾端，可以保留base class C struct的对象布局，因为允许在C程序中也能使用。

把vptr放在class object的前端，对于”在多重继承之下（下一节），通过指向class members的指针调用virtual function"，会带来一些帮助。否则，不仅”从class object起点开始量起“的offset必须在执行期备妥，甚至于class vptr之间的offset也必须备妥，当然，vptr放在前端，代价就是丧失了C语言兼容性。但是，这种丧失有多少意义？有多少程序会从一个C struct派生出一个具有多态性质的class呢？

#### c) 多重继承

![多重继承](./多重继承.png)
如图3.1b、图3.2a或图3.3。
你会看到base class和derived calss的objects都是从相同地址开始，其差异在于derived object比较大，用以多容纳它自己的nonstatic data members。
```c++
Point3d p3d;
Point2d *p = &p3d;
```
把一个derived calss object指定给base class的指针或引用，该操作不需要编译器去调停或修改地址，它很自然的发生，而且提供最佳的执行效率。

![vptr多重继承](./vptr放在class前端的继承.png)
图3.2b把vptr放在class object起始处，如果base class没有virtual function，而derived calss有，那么单一继承的自然多态就会被打破，这种情况下，把一个derived object转换成其base类型，就需要编译器介入，用以调整地址。

```c++
class Point2d {
public:
    // ...
protected:
    float _x, _y;
};

class Point3d: public Point2d {
public:
    // ...
protected:
    float _z;
};

class Vertex {
public:
    // ... 注意有virtual 接口，所以有vptr
protected:
    Vertex* next;
};

class Vertex3d: public Point3d, public Vertex {
public: 
    // ...
protected:
    float mumble;
};
```
多重继承的问题主要发生于derived class object和其第二或后继base class objects之间的转换；

```c++
extern void mumble(const Vertex&);
Vertex3d v;
// 将一个Vertex3d转换成一个Vertex，这是”不自然的”
mumble(v);
```
或是经由所支持的virtual function机制做转换。因支持“virtual function之调用操作“而引发的问题在4.2节讨论。

对于一个多重派生对象，将其地址指定给”最左端（也就是第一个）base class的指针”，情况将和单一继承时相同，因为二者都指向相同的起始地址，需付出的成本只有地址的指定操作而已）。至于第二个或后继的base class的地址指定操作，则需要将地址修改过：加上（或减去）鉴于中间的base class subobjects大小。

```c++
Vertex3d v3d;
Vertex *pv;
Point2d *p2d;
Point3d *p3d;

// 那么下面指定操作：
pv = &v3d;

// 需要这样内部转换：
pv = (Vertex*)(((char*)&v3d) + sizeof(Point3d));

// 而下面指定操作：
p2d = &v3d;
p3d = &v3d;

// 都只需要简单拷贝其地址就行了。如果有两个指针如下：
Vertex3d *pv3d;
Vertex   *pv;

// 那么下面指定操作：
pv = pv3d;

// 不能简单被转换为：
pv = (Vertex*)((char*)pv3d) + sizeof(Point3d);

// 因为如果pv3d为0，pv将获得sizeof(Point3d)的值，这是错误的！所以，对于指针，内部转换需要一个条件测试：
pv = pv3d ? (Vertex*)((char*)pv3d) + sizeof(Point3d) : 0;

// 至于reference，则不需要针对可能的0的值做防守，因为reference不可能参考到“无物”（no object)。
```

![数据布局：多重继承](./数据布局：多重继承.png)

C++ Standard并未要求Vertex3d中的base class Point3d和Vertex有特定的排列次序。

如果要存取第二个base calss中的一个data member，将会是怎样的情况？需要付出额外的成本吗？不，member的位置在编译时就固定了，因此存取member只是一个简单的offet运算，就像单一继承一样简单。


#### 虚拟继承（Virtual Inheritance）
多重继承语义上的副作用就是，它必须支持某种形式的"shared subobject继承“。一个典型例子是最早的iostream library：
```c++
class ios { ... }
class istream: public ios  { ... };
class ostream: public ios  { ... };
class iostream: 
    public istream, public ostream { ... };
```
![iostream多重继承](./iostream继承.png)

```c++
class ios { ... };
class istream: public virtual ios { ... };
class ostream: public virtual ios { ... };
class iostream: 
    public istream, public ostream { ... };
```
要在编译器中支持虚拟继承，实在是困难度颇高。实现技术的挑战在于，要找到一个足够有效地方法，将istream和ostream各自维护一个ios subobject，折叠成一个由iostream维护的单一ios subobject，并且还可以保存base class和derived class的指针之间的多态指定操作。


一般的方法如下所述。
Class中如果内含一个或多个virtual base class subobjects。像istream那样，将被分割成两部分：一个不变局部和一个共享局部。不变局部中的数据，不管后续如何衍化，总是拥有固定的offset（从object头部算起）。所以这部分数据可以直接存取；至于共享局部，所表现出的就是virtual base class subobjects。这一部分数据其位置会因为每次派生操作而变化。所以只能间接存取，各家编译器实现方式上差异在于间接存取方式不同。
```c++
class Point2d {
public:
    // ...
protected:
    float _x, _y;
};

class Vertex: public virtual Point2d {
public:
    // ...
private:
    Vertex *next;
};

class Point3d: public virtual Point2d {
public:
    // ...
protected:
    float _z;
};

class Vertex3d: public Vertex, public Point3d {
public:
    // ...
protected:
    float mumble;
};

```
![Vertex继承关系](./Vertex继承关系.png)

一般布局策略是先安排好derived class的不变部分，然后再建立其共享部分。

如何存取class的共享部分呢？cfront编译器会在每一个derived class object中安插一些指针，每个指针指向一个virtual base class，要存取继承而来的virtual base class members，可以使用相关指针间接完成。

```c++
void Point3d::operator+=(const Point3d &rhs) {
    _x += rhs._x;
    _y += rhs._y;
    _z += rhs._z;
}

//  在cfront策略下，这个运算符会被转换为：
__vbcPoint2d->_x += rhs.__vbcPoint2d->_x;
__vbcPoint2d->_y += rhs.__vbcPoint2d->_y; // vbc: virtual base class（共享部分？)
_z += rhs._z;


// 而一个derived class和一个base class的实例之间的转换，像这样：
Point2d *p2d = pv3d;

// 在cfront策略之下，会被变成：
Point2d *p2d = pv3d ? pv3d->__vbcPoint2d: 0;
```
这样实现模型的缺点：
1）每个对象必须针对每一个virtual base class背负一个额外的指针，然而我们希望class object有固定的负担，不会因为virtual base class的数目而有所变化，如何解决？
2）由于虚拟继承串链的加长，导致间接存取层次增加？假如有三层虚拟衍化，需要三次间接存取，然而我们希望有固定的存取时间？不因虚拟化的深度而改变？

`MetaWare`和其他编译器仍然使用cfront原始模型解决第二个问题？它们经由拷贝操作取得所有nested virtual base class指针，放到derived class object之中，解决”固定存取时间“的问题，虽然付出了空间上的代价。
![虚拟继承](./虚拟继承.png)

至于第一个问题，一般由两种方式解决：
a) 微软Mirocsoft编译器中每个class object中如果有一个或多个virtual base class就会由编译器安插一个指针，指向virtual base class table。至于真正的virtual base class指针，放在该表格中。

b) 在virtual function table中放置virtual base class的offset（而不是地址），下图显示这种实现模型。在新的Sun编译器中，virtual function table可经由正值或者负值来索引。如果正值，就显然索引到virtual function。如果是负值，则是索引到virtual base class offset。在这样的策略下，point3d的operator+=运算符必须被转换成以下形式：
```c++
(this + __vptr__Point3d[-1] /* 是偏移 */)->_x += (&rhs + rhs.__vptr__Point3d[-1])->_x;
(this + __vptr__Point3d[-1] /* 是偏移 */)->_y += (&rhs + rhs.__vptr__Point3d[-1])->_y;
_z += rhs._z;
```

![虚拟继承](./offset实现的虚拟继承.png)
(注意每个object对象都有一个__vptr)

经由一个非多态的class object来存取一个继承而来的virtual base calss的member，像这样：
```c++
Point3d origin;
// ...
origin._x;
```
可以被优化成一个直接存取操作，就好像一个经由对象调用的virtual function调用操作，可以在编译器被决议（resolve)完成一样。这次存取以及下一次存取之间，对象的类型不可能改变。

一般而言，virtual base class最有效的运行形式就是：一个抽象的virtual base class，没有任何data members。

#### 对象成员效率

旨在测试聚合、封装、以及继承所引发的额外负荷程度。


#### 指向Data Members的指针(Pointer to Data Members)

指向data members的指针，是一个有点神秘但是颇有用处的语言特性，特别是你需要调查class members的底层布局的话。这样的调查可以决定vptr放在class的起始出或是尾端，另一个用途是可以决定class中access section的次序，是一个神秘但有时有用的特性。
```c++
class Point3d {
public:
    virtual ~Point3d();
    // ...
protected:
    static Point3d origin;
    float x, y, z;
};
```
每一个Point3d class object都有三个坐标值，依次为x, y, z，以及一个vptr，至于static data member origin，将被放置在class object之外，唯一不同的是vptr的位置。
```c++
// 取一个nonstatic data member的地址 => 得到在class中的offset
&Point3d::z  => float Point3d::*：返回在class中的offset
&origin::z   => float *： 返回绑定在class object上的data member的地址
```

#### 指向"Member的指针”的效率问题

为每一个“member存取操作”加上一层间接性（经由已绑定的指针），会使执行时间多一倍不止。以指向“member的指针“来存取数据，在一次用掉了双倍时间，要把”指向member的指针”绑定到class object身上，需要额外的把offset减1。

![指向data member](./虚拟继承对指向datamember的指针存取方式的影响.png)

由于继承的data member是直接存放在class object之中，所以继承的引入一点也不会影响这些码的效率。虚拟继承所带来的主要冲击是，它妨碍了优化的有效性。为什么呢？在两个编译器（CC和NCC）
中，每一层虚拟继承都导入一个额外的层次间接性。每次存取Point::x，像这样：
```c++
pB.*bx
// 会被转换为
&pB->__vbcPoint + (bx - 1); // 注意第一层可能被优化为直接存取

// 而不是最直接的：
&pB + (bx - 1)
```
额外的间接性会降低“把所有处理都搬移到缓存器中执行”的优化能力。
