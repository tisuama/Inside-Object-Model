## 构造函数语意学

ARM => C++ Annotated Reference Manual

C++ Standard在Section 12.1说
>	对于class X，如果没有热河user-declared constructor，那么会有一个default constructor被暗中(implicitly)声明出来，它将是一个trivial(没啥用的)constructor。

### Nontrivaial default constructor

Nontrivaial default constructor在ARM的术语中就是编译器所需要的哪种，必要的化会由编译器合成。
