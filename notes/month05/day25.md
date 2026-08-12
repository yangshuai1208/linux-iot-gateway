# 第五个月 Day25：RAII 与智能指针

## RAII

资源生命周期与对象生命周期绑定。

构造函数获取资源，
析构函数释放资源。

适用于：

- 文件
- Socket
- 串口
- mutex
- 动态内存

## unique_ptr

唯一所有权。

不能复制，可以通过 std::move 转移。

适用于一个对象明确只由一个模块管理的情况。

例如：

GatewayWorker
→ unique_ptr<SerialPort>

## shared_ptr

共享所有权。

内部通过引用计数管理生命周期。

最后一个 shared_ptr 消失时对象才销毁。

## weak_ptr

观察 shared_ptr 管理的对象，但不增加引用计数。

使用 lock() 获取临时 shared_ptr。

适用于：

- Observer
- Callback
- 避免循环引用

## std::move

本身主要用于把对象转换成可移动状态，
让资源可以从一个对象转移到另一个对象。

unique_ptr 必须通过 move 转移所有权。

## 项目映射

SerialPort / Socket
→ 优先 unique_ptr 或直接 RAII 对象

共享配置
→ 必要时 shared_ptr

非拥有型观察
→ weak_ptr

## 原则

不要为了使用智能指针而全部使用 shared_ptr。

优先明确资源所有权。