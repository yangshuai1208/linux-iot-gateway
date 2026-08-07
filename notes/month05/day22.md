# Day22 Sequence 去重与重复请求处理

## 问题

命令执行成功，但 ACK 丢失：

Gateway
→ timeout
→ retry

如果设备直接重新执行，就可能产生重复动作。

## 解决方案

使用 sequence 判断请求是否已经处理。

第一次：

seq=3
→ 执行
→ 保存 ACK

第二次：

seq=3
→ 发现已经处理
→ 不重复执行
→ 重新发送缓存 ACK

## ackCache

使用：

map<sequence, AckFrame>

缓存已经处理过的请求结果。

## 实验

received=2
executed=1
duplicates=1

说明相同 sequence 重传时不会重复执行业务。

## 幂等

重复请求不应产生额外业务副作用。

sequence 去重是实现可靠通信的重要基础之一。

## 当前边界

当前仍为 Linux C++ 模拟 Device。

尚未真正移植到 STM32。