# 第五个月 Day23：C++ 网关综合复盘

## 一、并发知识链

Day15：
std::thread + atomic

Day16：
mutex + lock_guard + thread-safe LogManager

Day17：
queue + condition_variable + Producer/Consumer

Day18：
将 Producer/Consumer、ProtocolParser、LogManager 集成为网关并发处理链路

## 二、通信协议知识链

Day19：
sequence + ACK / ERROR

Day20：
wait_for + timeout + retry

Day21：
binary frame + checksum + stream parser

Day22：
sequence duplicate detection + cached ACK

## 三、完整网关链路

UART/MQTT Input
→ queue
→ Consumer
→ ProtocolParser
→ Command
→ Binary Frame
→ Device
→ ACK

ACK timeout
→ retry same sequence
→ Device duplicate detection
→ skip execution
→ resend cached ACK

## 四、线程同步原则

1. STL queue 本身不线程安全
2. mutex 保护共享状态
3. condition_variable 用于等待和通知
4. wait 使用 unique_lock
5. 临界区尽量短
6. 不存在并发冲突的对象不需要强行加锁

## 五、可靠通信原则

1. 帧头用于同步
2. length 用于判断完整帧
3. sequence 用于请求响应关联
4. checksum 用于检错
5. ACK 用于反馈处理结果
6. wait_for 实现超时
7. retry 必须有限次数
8. 同一次 retry sequence 不变
9. 重复 sequence 不重新执行
10. 缓存 ACK 可以直接回复重复请求

## 六、当前项目边界

当前完成的是 Linux C++ 模拟环境下的并发与可靠通信验证。

尚未完成：

- 全部模块接入真实 UART
- STM32 二进制 ACK
- CRC
- sequence 滑动窗口
- 持久化去重
- 设备重启后的 session 管理