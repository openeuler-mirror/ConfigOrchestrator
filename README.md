# Config Orchestrator

## 构建

安装依赖：

```bash
cd deps
./build-all
sudo ./build-all install
```

构建项目：

```bash
$ mkdir -p build && cd build

$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ cmake --build . --parallel

$ ./controlpanel
```

## 测试

我们使用 gtest 进行测试。你可以通过以下命令测试所有用例：

```bash
ctest
```
## 如何添加配置


## 代码规范

- 使用 fmt 进行日志输出
- 尽可能使用智能指针
- 使用 gtest 进行测试
