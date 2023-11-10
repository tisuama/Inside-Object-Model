## Data 语意学
一个空的class如：
```c++
// sizeof X == 1
class X {};
```
事实上，X并不是空的，他有一个隐晦的1 byte，那是编译器安插进去的一个char，这使得这个class的两个objects得以在内存配置独一无二的内存地址：
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

记住一个virtual base class subject只会在derived class中存在一分实体，不管他在继承体系中出现了多少次，class A的大小由以下几点决定：
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

然而，反对对于member function的argument list并不为真，Argument list中的名称还是会在他们第一次遭遇时被释放的决议（resolve)完成，因此在extern 和 nested type names之间的非直觉绑定操作还会发生。

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
