# 第五个月 Day28：C++ 与 Linux Gateway 面试收口

## 一、今日手写

### ProtocolParser

复习：

- string
- enum class
- map
- transform
- lambda
- const reference

### Producer Consumer

复习：

- std::thread
- queue
- mutex
- unique_lock
- condition_variable
- predicate

## 二、最容易忘的知识

1. std::move 本身不移动资源
2. unique_ptr 不能复制但可以移动
3. shared_ptr 表达共享所有权
4. weak_ptr 不拥有对象
5. Lambda 引用捕获要考虑生命周期
6. queue 本身不保证线程安全
7. condition_variable 的 notify 不等于业务条件成立
8. Consumer 退出必须同时满足 finished 和 queue empty

## 三、Gateway 可靠通信

Request
→ sequence
→ binary frame
→ Device
→ ACK

ACK timeout
→ retry same sequence
→ duplicate detection
→ skip duplicate execution
→ cached ACK

## 四、项目边界

C++ 并发、ACK、二进制协议、去重等模块主要为 Linux 模拟验证，
没有包装成已经全部部署到 STM32 实际链路。