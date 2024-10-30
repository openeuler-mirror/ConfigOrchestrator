# How to add configuration for control panel

## 错误处理框架

在现代 C++ 中，使用 exception 系统来处理错误，好处是可以将前端显示和后端测试的错误捕获和报告结合起来，坏处是要求后端函数是异常安全的。

```C++
auto msg = fmt::format("...");
throw std::invalid_argument(msg); // 对于错误的后端函数调用
throw std::out_of_range(msg); // 越界
throw std::runtime_error(msg); // 运行时的问题，比如要处理的设置失败
```

后端函数应当提供这样的保证：

- 抛出异常的时候不能泄漏任何资源（用 RAII 管理）
- 不允许数据损坏 (如先增加了 ref 个数，然后 new 失败了)

对于前端而言，会在 display 中设置兜底异常，如果有未捕获的异常，将以 Error Dialog 的形式展示。
