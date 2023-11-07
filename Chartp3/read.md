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
3. Alignment限制，class Y和Z大小截止目前为5byte（32位？）大部分机器上会受到alignment限制

那么你期望A的大小是什么？

