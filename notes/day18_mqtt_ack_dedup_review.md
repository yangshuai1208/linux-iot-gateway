# Day18 MQTT 可靠通信、ACK 超时重传与 Sequence 去重复习

## 一、今日目标

今天主要复习 AIoT 项目中的 MQTT 与可靠通信机制，并完成 Linux C 模拟代码实验。

重点内容：

- MQTT QoS 0 / 1 / 2
- TCP 与 MQTT QoS 的区别
- Keep Alive
- Retain
- Last Will
- MQTT 断线重连
- JSON 字段校验
- MQTT PUBACK 与 STM32 执行 ACK
- Sequence 序列号
- ACK 超时重传
- 重复请求去重
- 最大重试次数
- 指数退避与随机抖动
- 项目真实实现边界

今日完成两个代码实验：

```text
test/day18_sequence_dedup.c
test/day18_ack_retry.c