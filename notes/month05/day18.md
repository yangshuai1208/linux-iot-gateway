# 第五个月 Day18：C++ 多线程网关最小闭环

## 今日目标

将 ProtocolParser、线程安全 LogManager 和生产者消费者模型进行集成，实现模拟输入下的 Linux IoT Gateway C++ 多线程最小闭环。

## 今日完成内容

1. 创建 gateway_pipeline_demo
2. 使用 queue 保存原始控制命令
3. Producer 模拟 UART/MQTT 输入
4. 使用 mutex 保护命令队列
5. 使用 condition_variable 进行等待和通知
6. Consumer 调用 ProtocolParser
7. 文本命令转换为 Command 枚举
8. Command 转换为 HAND_* 执行命令
9. 非法命令安全拒绝
10. Producer 与 Consumer 共享线程安全 LogManager
11. 使用 PipelineStats 统计合法与非法命令
12. 安全 join 所有工作线程
13. 更新 README 与 GitHub

## 当前数据流

```text
模拟 UART/MQTT 输入
→ std::queue<string>
→ condition_variable
→ ProtocolParser
→ Command
→ HAND_*
→ LogManager
```

## Producer

Producer 模拟通信接收线程。

职责：

```text
生成原始命令
→ push queue
→ notify_one
```

当前测试：

```text
OPEN
 grab\r\n
JUMP
STOP
release\n
```

## Consumer

Consumer 模拟协议处理线程。

职责：

```text
wait
→ pop queue
→ ProtocolParser
→ 合法性检查
→ HAND_* 映射
→ 日志记录
```

## 非法输入

```text
JUMP
→ Command::Unknown
→ isValid=false
→ WARNING
→ 不产生真实执行动作
```

## 线程同步

commandQueue 和 inputFinished 由同一把 queueMutex 保护。

Consumer 使用：

```cpp
condition.wait(
    lock,
    predicate);
```

队列有数据或者输入结束时才能继续。

## LogManager

Producer 和 Consumer 都会调用同一个 LogManager。

Day16 已使用 mutex 和 lock_guard 保护内部：

- ofstream
- vector
- map
- console

因此可以安全用于当前双线程 Demo。

## ProtocolParser

当前只有 Consumer 使用 ProtocolParser，因此暂时不需要额外 mutex。

同步应该根据真正的并发共享关系设计，而不是所有对象都加锁。

## PipelineStats

当前只有 Consumer 修改 accepted 和 rejected。

main 在 consumer join 后读取。

所以不存在并发访问，不需要额外 mutex 或 atomic。

## 实验结果

测试命令共 5 条：

```text
合法 4
非法 1
```

预期：

```text
accepted=4
rejected=1
```

合法命令：

```text
OPEN → HAND_OPEN
GRAB → HAND_GRAB
STOP → HAND_STOP
RELEASE → HAND_RELEASE
```

非法：

```text
JUMP → rejected
```

## 当前工程边界

当前属于模拟输入下的 C++ 并发网关 Demo。

尚未正式接入：

- UART read
- MQTT subscribe
- TCP socket
- STM32 实际串口发送

不能描述为完整网关已经全部完成 C++ 重构。

## Day13～Day18 演进

```text
ProtocolParser
→ thread
→ mutex
→ thread-safe LogManager
→ condition_variable
→ producer-consumer
→ concurrent gateway pipeline
```

## 今日收获

开始把零散 C++ 知识转变成实际 Linux IoT Gateway 架构能力。

## 明日计划

继续增强网关并发闭环，重点加入结构化命令消息、sequence 和执行结果/ACK 概念，同时整理并发项目面试话术。