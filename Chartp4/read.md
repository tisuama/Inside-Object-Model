## Function 语意学

C++支持三种类型member function: static、nonstatic和virtual，每一种类型被调用的方式都不相同，其中有两个原因：1）直接存取nonstatic 数据 2）它被声明为const。使得，static member function不可能做到这两点。

原始的”C with Classes“只支持nonstatic member functions。Virtual 函数在20世纪80年代中期被加进来，并且受到许多质疑。Static member functions是最后被引入的一种函数类型，在1987年Useix C++研讨会的厂商研习营中被正式提议加入C++中，并由cfront2.0实现出来。

#### Nonstatic  Memeber Function（非静态成员函数）

