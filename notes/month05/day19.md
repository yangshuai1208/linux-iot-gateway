# 第五个月 Day19：结构化命令与 ACK 状态反馈

## 今日目标

在 Day18 多线程网关闭环基础上增加 RawCommand、sequence、ResultCode 和 CommandResult，使请求与处理结果可以建立对应关系。

## 数据流

```text
RawCommand
→ queue
→ ProtocolParser
→ Command
→ 模拟执行结果
→ CommandResult
→ ACK / ERROR
```

## RawCommand

```cpp
struct RawCommand
{
    std::uint32_t sequence;
    std::string source;
    std::string payload;
};
```

相比单纯 string，结构化命令可以携带：

- 请求序号
- 数据来源
- 原始命令

## sequence

sequence 用于关联请求与响应。

例如：

```text
seq=3 JUMP
→ ERROR seq=3 INVALID_COMMAND
```

客户端可以明确知道反馈对应的是哪一条命令。

## ResultCode

```text
0    OK
1001 INVALID_COMMAND
1002 EXECUTION_FAILED
```

统一错误码比直接返回不同字符串更方便程序判断。

## CommandResult

包含：

- sequence
- result code
- command
- executor command
- message

用于表示一次命令处理结果。

## ACK

合法命令：

```text
ACK
seq=1
code=0
OPEN
HAND_OPEN
```

当前 ACK 只表示网关侧完成解析和执行指令映射。

尚不是 STM32 的真实硬件执行反馈。

## ERROR

非法命令：

```text
ERROR
seq=3
code=1001
INVALID_COMMAND
```

非法输入不会产生真实执行动作。

## PipelineStats

统计：

```text
received
accepted
rejected
responses
```

当 Producer 和 Consumer 都需要修改统计数据时，应使用 mutex 或 atomic 等同步方式。

## 与 Day18 的区别

Day18：

```text
string
→ Command
→ HAND_*
```

Day19：

```text
RawCommand
→ sequence
→ Command
→ CommandResult
→ ACK / ERROR
```

## 当前边界

尚未实现：

- 真实 STM32 ACK
- UART 应答帧
- 超时重发
- sequence 去重
- 校验码
- 执行失败反馈
- 网络响应发送

## 后续方向

```text
Gateway 下发命令
→ STM32执行
→ ACK(seq)
→ Gateway 匹配 sequence
→ 超时判断
→ 重发/错误上报
```