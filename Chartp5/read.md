## 构造、解构、拷贝语意学

考虑下面abstract base class声明：
```c++
class Abstruact_base {
public:
    virtual ~Abstruact_base() = 0;
    virtual void interface() const = 0;
    virtual const char* mumble() const { return _memble; }
protected:
    char* _mumble;
};

```
