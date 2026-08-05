·# Day20 ACK 超时与重试

## 数据流

发送命令
→ sequence
→ PendingRequest
→ 等待 ACK
→ ACK 匹配 sequence
→ 成功

如果超时：

TIMEOUT
→ RETRY
→ 再等待 ACK
→ 成功或 FAILED

## wait_for

condition_variable::wait_for 可以设置最大等待时间，
适合 ACK、网络响应、串口响应等超时检测。

## sequence

ACK 不能只判断“有没有回来”，
还必须判断返回的 sequence 是否等于当前请求 sequence。

## retry

当前最多重试 1 次：

第一次发送失败
→ Retry
→ 第二次仍失败
→ FAILED

## 当前边界

当前使用模拟设备线程，不是真实 STM32 ACK。

尚未实现：

- 真实 UART
- CRC
- 重复 sequence 去重
- 指数退避
- 多个并发 PendingRequest