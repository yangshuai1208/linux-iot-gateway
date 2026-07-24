# 第五个月 Day9：LogManager 构造函数和析构函数

## 今日目标

为 LogManager 增加构造函数、析构函数和文件日志功能，理解对象生命周期和 RAII 资源管理。

## 今日完成内容

1. 学习构造函数
2. 学习析构函数
3. 学习成员初始化列表
4. 学习 explicit 关键字
5. 学习 std::ofstream 文件输出流
6. 使用构造函数打开日志文件
7. 使用析构函数刷新并关闭日志文件
8. 实现终端与文件双通道日志
9. 使用追加模式保存历史日志
10. 更新 Makefile、README 和 .gitignore
11. 完成编译运行和 GitHub 提交

## 构造函数

构造函数在创建对象时自动调用。

```cpp
explicit LogManager(const std::string &filePath);
```

构造函数特点：

- 名称与类名相同
- 没有返回值
- 可以接收参数
- 用于初始化对象资源

本项目在构造函数中打开日志文件。

## 析构函数

析构函数在对象销毁时自动调用。

```cpp
~LogManager();
```

析构函数特点：

- 名称为类名前加 `~`
- 没有返回值
- 没有参数
- 一个类只有一个析构函数
- 用于释放对象占用的资源

本项目在析构函数中刷新并关闭日志文件。

## 对象生命周期

```text
进入 main()
    ↓
创建 logger 对象
    ↓
调用构造函数
    ↓
打开日志文件
    ↓
调用 log()
    ↓
main() 即将结束
    ↓
logger 离开作用域
    ↓
自动调用析构函数
    ↓
关闭日志文件
```

## 成员初始化列表

```cpp
LogManager::LogManager(const std::string &filePath)
    : filePath_(filePath),
      logFile_(filePath, std::ios::app)
```

初始化列表用于直接初始化成员变量。

它比在构造函数体内先创建成员再赋值更加规范和高效。

## std::ofstream

`std::ofstream` 是 C++ 文件输出流。

本项目使用：

```cpp
std::ofstream logFile_;
```

打开方式：

```cpp
std::ios::app
```

表示使用追加模式，不覆盖之前的日志。

## explicit

构造函数前使用：

```cpp
explicit
```

可以防止字符串被编译器自动隐式转换为 LogManager 对象，使类型转换更明确。

## RAII

RAII 的核心思想是：

```text
资源获取即初始化
```

对象创建时获取资源，对象销毁时自动释放资源。

本项目中：

```text
构造函数 → 打开文件
析构函数 → 关闭文件
```

即使以后函数提前返回，只要对象正常离开作用域，析构函数仍会被调用，减少文件忘记关闭的问题。

## log() 为什么不再是 const

Day8 中 log() 只向终端输出，不修改对象状态，因此可以写成 const 成员函数。

Day9 中 log() 需要向 `logFile_` 写入数据，会改变文件流状态，所以移除了成员函数末尾的 const。

## 实验现象

终端输出：

```text
[INFO] Log file opened: log_manager_demo.log
[INFO] Linux IoT Gateway started
[WARNING] MQTT Broker is not connected
[ERROR] Failed to parse sensor data
[INFO] Log file closed: log_manager_demo.log
```

日志文件成功保存三条业务日志。

再次运行程序后，新日志追加到文件末尾，没有覆盖原有内容。

## 今日收获

构造函数和析构函数让资源管理与对象生命周期绑定。

相比手动调用 open 和 close，RAII 可以减少异常、提前返回或忘记释放资源导致的问题。

## 明日计划

学习引用和 this 指针，优化 LogManager 的参数传递，并使用 this 明确访问当前对象的成员变量。