# 第五个月 Day24：Linux IoT Gateway 面试强化

## 今日目标

从“代码实现”转向“项目讲解与面试追问”。

## 项目核心链路

UART / Mock
→ Data Source
→ Parser
→ SensorData
→ JSON
→ Log / MQTT

## C++ 实验增强

ProtocolParser
→ LogManager
→ Producer / Consumer
→ sequence
→ ACK
→ timeout
→ retry
→ binary frame
→ duplicate detection

## 关键设计

### 为什么生产者消费者

解耦通信 I/O 与协议业务处理，避免解析和日志耗时阻塞数据接收。

### 为什么 condition_variable

没有数据时阻塞 Consumer，避免持续轮询。

### 为什么 sequence

用于请求响应匹配、超时重试和重复请求检测。

### 为什么 ACK timeout 不等于执行失败

设备可能已经执行成功，只是 ACK 丢失。

### 为什么 Retry 使用相同 sequence

设备需要通过 sequence 判断这是旧请求重传，而不是新业务请求。

### 为什么需要 ACK Cache

重复请求到来时不再次执行业务，而是返回原执行结果。

## 当前项目边界

C++ 并发和可靠通信部分属于 Linux 模拟验证，
没有包装成全部部署到真实 STM32。