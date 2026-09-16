# 第七阶段 Day1 学习记录

## 1. C 协议状态机

实现文件：

```text
practice/day01/protocol_parser.c

协议格式：

HEAD | LEN | PAYLOAD | CHECKSUM | TAIL

其中：

HEAD = 0xAA
TAIL = 0x55
MAX_PAYLOAD = 32

使用状态机：

WAIT_HEAD
→ WAIT_LEN
→ WAIT_PAYLOAD
→ WAIT_CHECKSUM
→ WAIT_TAIL
重点知识
当前状态决定收到的 byte 应该如何解释
非法长度需要恢复到 WAIT_HEAD
注意 switch fall-through
注意数组边界
CHECKSUM 为简单累加校验，不等同于 CRC
可处理半包和连续帧
易错点
= 与 ==
byte == HEAD，不是 state == HEAD
payload_index >= payload_len
for 循环变量必须初始化
switch case 必须注意 break
2. C++17 线程安全队列

实现文件：

practice/day01/thread_safe_queue.cpp

使用：

std::queue
std::mutex
std::lock_guard
std::unique_lock
std::condition_variable
std::thread
核心流程

生产者：

加锁
→ push
→ 解锁
→ notify_one

消费者：

加锁
→ 条件不满足时 wait
→ 自动释放 mutex
→ 被唤醒
→ 重新获得 mutex
→ 检查条件
→ front / pop

退出：

stopped_ = true
→ notify_all
→ 消费者检查退出条件
→ return false
→ join
面试点

condition_variable 必须配合条件判断，因为可能存在虚假唤醒，而且线程被唤醒后，共享条件也可能已经被其他线程改变。

lock_guard 适合简单 RAII 加锁。

unique_lock 更灵活，可以主动释放和重新获得锁，因此通常与 condition_variable 配合使用。

3. 今日 Linux 复习薄弱点

需要继续复习：

select 与 epoll
EINTR / EAGAIN
VMIN / VTIME
condition_variable 安全退出
lock_guard / unique_lock
GDB core dump 定位
Makefile 增量编译