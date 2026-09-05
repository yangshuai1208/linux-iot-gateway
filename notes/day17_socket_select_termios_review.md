# Day17：Linux Socket、select/epoll、termios与网关过载复习

## 一、今日完成内容

Day17围绕Linux IoT Gateway的网络通信、I/O多路复用和串口阻塞问题展开。

完成内容：

- 复习TCP与UDP的核心区别。
- 掌握`send()`和`recv()`的返回值处理。
- 实现能够处理短写和`EINTR`的`SendAll()`。
- 理解TCP半包、粘包和应用层分帧。
- 使用`select()`实现单线程多客户端Echo Server。
- 掌握`fd_set`和`max_fd`的维护。
- 对比`select()`与`epoll`。
- 理解epoll的LT和ET触发模式。
- 复习Linux串口Raw模式、115200 8N1和`VMIN/VTIME`。
- 分析有界队列过载、MQTT回调阻塞和串口线程退出问题。
- 完成Day17判断题，成绩为13/15。

新增代码：

```text
test/day17_send_all.c
test/day17_select_echo_server.c
```

验证结果：

```text
Day17 send_all tests passed
select Echo Server多客户端测试通过
```

---

## 二、TCP与UDP

### 1. TCP特点

TCP提供：

- 面向连接。
- 可靠传输。
- 按序到达。
- 重传机制。
- 流量控制。
- 拥塞控制。
- 全双工字节流。

TCP不保留应用层消息边界。

错误理解：

```text
一次send()一定对应对端的一次recv()
```

实际可能出现：

```text
发送端send(A)
发送端send(B)

接收端可能：
recv()得到A+B
也可能：
recv()得到A的一部分
recv()得到A剩余部分+B
```

因此TCP应用必须自行设计分帧协议。

### 2. UDP特点

UDP提供无连接的数据报传输：

- 保留数据报边界。
- 不保证一定到达。
- 不保证顺序。
- 不保证不重复。
- 协议开销较小。
- 支持广播和组播。

UDP保留消息边界，但不等于UDP可靠。

如果业务需要可靠UDP，需要自行增加：

- sequence序号。
- ACK。
- 超时重传。
- 重复请求去重。
- 校验。
- 乱序处理。

### 3. 项目中的选择

| 场景 | 常用协议 | 原因 |
|---|---|---|
| MQTT上传 | TCP | 需要可靠、有序传输 |
| 网关控制客户端 | TCP | 命令不能随意丢失 |
| 局域网设备发现 | UDP | 数据量小，便于广播 |
| 高频姿态数据 | 根据业务选择 | 权衡实时性、可靠性和复杂度 |

---

## 三、`send()`返回值

```c
ssize_t sent = send(socket_fd,
                    data,
                    length,
                    MSG_NOSIGNAL);
```

返回值含义：

| 返回值 | 含义 | 处理方法 |
|---:|---|---|
| `sent > 0` | 本次实际发送的字节数 | 累加发送偏移 |
| `sent == 0` | 没有取得发送进展 | 返回失败，防止死循环 |
| `sent < 0 && errno == EINTR` | 被信号中断 | 保持偏移并重试 |
| `sent < 0 && errno == EAGAIN` | 非阻塞Socket暂时不可写 | 等待可写事件 |
| 其他负值 | 真实发送错误 | 记录错误并断开或恢复 |

即使是阻塞Socket，`send()`也不保证一次发送完整数据。

---

## 四、完整发送函数`SendAll()`

```c
static bool SendAll(int socket_fd,
                    const uint8_t *data,
                    size_t length)
{
    size_t total_sent = 0U;

    if ((socket_fd < 0) ||
        ((data == NULL) && (length > 0U)))
    {
        return false;
    }

    while (total_sent < length)
    {
        ssize_t sent =
            send(socket_fd,
                 data + total_sent,
                 length - total_sent,
                 MSG_NOSIGNAL);

        if (sent > 0)
        {
            total_sent += (size_t)sent;
        }
        else if ((sent < 0) && (errno == EINTR))
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}
```

循环中的三个关键值：

```text
已发送字节数：total_sent
下次发送地址：data + total_sent
剩余发送长度：length - total_sent
```

必须保持：

```text
0 <= total_sent <= length
```

### `MSG_NOSIGNAL`

向已关闭连接发送数据时，Linux可能产生`SIGPIPE`，默认行为可能终止整个进程。

使用：

```c
MSG_NOSIGNAL
```

可以阻止本次`send()`触发`SIGPIPE`。

但它不会把发送变成成功，程序仍然需要检查：

```text
EPIPE
ECONNRESET
ENOTCONN
```

---

## 五、`SendAll()`代码错题

### 错误1：只把偏移加1

错误：

```c
total_sent++;
```

正确：

```c
total_sent += (size_t)sent;
```

`sent`表示本次真正发送的字节数，而不是一次系统调用。

### 错误2：在`EINTR`分支中再次声明`sent`

错误：

```c
else if ((sent < 0) && (errno == EINTR))
{
    ssize_t sent = send(...);
}
```

问题：

- 新变量遮蔽外层变量。
- 新的返回值没有得到统一处理。
- 逻辑重复。
- 容易漏掉偏移更新。

正确：

```c
else if ((sent < 0) && (errno == EINTR))
{
    continue;
}
```

循环会使用相同偏移重新调用`send()`。

### 阻塞版与非阻塞版的区别

本次`SendAll()`是阻塞版。

如果Socket设置为非阻塞，遇到：

```text
EAGAIN
EWOULDBLOCK
```

不能在原位置持续循环，否则会造成CPU忙等并阻塞事件循环。

非阻塞版本需要保存：

- 待发送缓冲区。
- 当前偏移。
- 剩余长度。
- 当前发送状态。

随后等待：

```text
select写就绪
或
epoll的EPOLLOUT
```

再继续发送。

---

## 六、`recv()`返回值

```c
ssize_t received =
    recv(socket_fd,
         buffer,
         sizeof(buffer),
         0);
```

| 返回值 | 含义 |
|---:|---|
| `> 0` | 实际收到的字节数 |
| `== 0` | 对端已经有序关闭连接 |
| `< 0 && errno == EINTR` | 系统调用被信号中断 |
| `< 0 && errno == EAGAIN` | 非阻塞Socket暂时没有数据 |
| 其他负值 | 真实错误 |

特别注意：

```text
recv() == 0
```

不是“暂时没有数据”，而是对端关闭了连接。应该：

```text
从监听集合删除
→ close描述符
→ 清理连接状态
```

---

## 七、TCP半包与粘包

### 1. 半包

一条完整消息被分成多次`recv()`收到。

例如：

```text
第一次：HAND_
第二次：OPEN\r\n
```

### 2. 粘包

多条消息被一次`recv()`收到。

例如：

```text
HAND_OPEN\r\nHAND_STOP\r\n
```

### 3. 正确处理流程

```text
新数据追加到累计缓冲区
→ 检查是否存在完整帧
→ 解析一条完整帧
→ 从缓冲区移除已处理帧
→ 继续检查下一条完整帧
→ 只保留最后的不完整尾部
```

必须使用循环解析，因为一次接收可能包含多条完整消息。

常见分帧方法：

- 固定长度。
- `\r\n`分隔符。
- 长度字段。
- 帧头、长度、Payload、校验、帧尾。

### 本次错点

如果一次`recv()`收到两条完整帧，应当在当前循环中把两条都处理完。

不能处理第一条后，把完整的第二条留到下一次网络数据到来，否则会：

- 增加延迟。
- 让缓冲区堆积。
- 在没有新数据时无法继续处理第二条。

---

## 八、`select()`工作原理

`select()`允许一个线程同时等待多个文件描述符。

本次服务器维护：

```text
监听Socket
多个客户端Socket
master_set长期集合
read_set临时集合
max_fd最大描述符
```

### 1. `fd_set`宏

```c
FD_ZERO(&master_set);
FD_SET(listen_fd, &master_set);
FD_ISSET(descriptor, &read_set);
FD_CLR(descriptor, &master_set);
```

作用：

| 宏 | 作用 |
|---|---|
| `FD_ZERO` | 清空描述符集合 |
| `FD_SET` | 添加描述符 |
| `FD_ISSET` | 判断描述符是否存在或就绪 |
| `FD_CLR` | 删除描述符 |

### 2. 为什么复制`fd_set`

```c
fd_set read_set;
read_set = master_set;
```

然后：

```c
int ready_count =
    select(max_fd + 1,
           &read_set,
           NULL,
           NULL,
           NULL);
```

`select()`会修改传入的集合，只留下本轮就绪的描述符。

所以：

```text
master_set：长期保存全部描述符
read_set：每轮交给select修改
```

不能直接把长期使用的`master_set`交给`select()`后继续复用。

### 3. 为什么是`max_fd + 1`

`select()`第一个参数不是描述符数量，而是检查范围的上界。

```text
检查范围：0到max_fd
参数值：max_fd + 1
```

正确调用：

```c
select(max_fd + 1,
       &read_set,
       NULL,
       NULL,
       NULL);
```

---

## 九、接受新客户端

```c
int client_fd =
    accept(listen_fd, NULL, NULL);

if (client_fd < 0)
{
    /* 处理错误 */
}

if (client_fd >= FD_SETSIZE)
{
    close(client_fd);
}
else
{
    FD_SET(client_fd, &master_set);

    if (client_fd > max_fd)
    {
        max_fd = client_fd;
    }
}
```

新客户端连接后必须完成：

```text
FD_SET加入master_set
→ 更新max_fd
```

---

## 十、删除描述符

```c
static void RemoveDescriptor(int descriptor,
                             fd_set *master_set,
                             int *max_fd)
{
    if ((descriptor < 0) ||
        (master_set == NULL) ||
        (max_fd == NULL))
    {
        return;
    }

    FD_CLR(descriptor, master_set);
    close(descriptor);

    if (descriptor == *max_fd)
    {
        while ((*max_fd >= 0) &&
               !FD_ISSET(*max_fd, master_set))
        {
            (*max_fd)--;
        }
    }
}
```

删除步骤：

```text
FD_CLR从集合删除
→ close关闭描述符
→ 如果删除的是max_fd
→ 向下寻找新的最大描述符
```

---

## 十一、`select()`代码错题

### 错误1：`select()`参数错误

错误：

```c
select(max_fd + 1, read_set, write, NULL);
```

正确：

```c
int ready_count =
    select(max_fd + 1,
           &read_set,
           NULL,
           NULL,
           NULL);
```

`select()`一共有五个参数：

```c
select(nfds,
       readfds,
       writefds,
       exceptfds,
       timeout);
```

### 错误2：把集合赋值成描述符

错误：

```c
master_set = client_fd;
```

正确：

```c
FD_SET(client_fd, &master_set);
```

### 错误3：没有更新`max_fd`

错误：

```c
if (client_fd > max_fd)
{
    max_fd;
}
```

正确：

```c
if (client_fd > max_fd)
{
    max_fd = client_fd;
}
```

### 错误4：多取了一次地址

函数参数已经是：

```c
fd_set *master_set
```

错误：

```c
FD_CLR(descriptor, &master_set);
```

这会得到`fd_set **`。

正确：

```c
FD_CLR(descriptor, master_set);
```

### 错误5：把指针和整数比较

错误：

```c
if (descriptor == max_fd)
```

正确：

```c
if (descriptor == *max_fd)
```

### 错误6：运算符优先级错误

错误：

```c
*(max_fd)--;
```

后缀`--`优先级高于`*`，这会移动指针。

正确：

```c
(*max_fd)--;
```

含义是减少`max_fd`指向的整数。

---

## 十二、`select()`的限制

`select()`存在以下限制：

- 受`FD_SETSIZE`限制。
- 每轮需要复制描述符集合。
- 每轮需要从0扫描到`max_fd`。
- 描述符越大，扫描范围越大。
- 不适合大量并发连接。

它适合：

- 学习I/O多路复用。
- 管理少量连接。
- 简单控制服务器。
- 小型网关实验。

Linux高并发服务通常使用`epoll`。

---

## 十三、epoll基础

基本流程：

```text
epoll_create1()
→ epoll_ctl()注册事件
→ epoll_wait()等待就绪事件
```

与`select()`相比：

| 对比项 | select | epoll |
|---|---|---|
| 描述符集合 | 每轮传入 | 注册到内核 |
| 就绪结果 | 扫描整个范围 | 返回就绪事件 |
| 数量限制 | 受`FD_SETSIZE`影响 | 通常受系统资源限制 |
| 适用场景 | 少量连接 | 大量Linux连接 |

---

## 十四、LT与ET

### 1. LT水平触发

只要文件描述符仍然处于就绪状态，`epoll_wait()`就会继续通知。

即使一次没有读完，下一轮通常还会收到通知。

### 2. ET边缘触发

主要在状态从“未就绪”变成“就绪”时通知。

ET模式通常要求：

```text
非阻塞Socket
→ 收到事件
→ 循环recv()
→ 一直读取到EAGAIN/EWOULDBLOCK
```

典型代码：

```c
for (;;)
{
    ssize_t received =
        recv(socket_fd,
             buffer,
             sizeof(buffer),
             0);

    if (received > 0)
    {
        /* 处理数据 */
    }
    else if ((received < 0) &&
             (errno == EINTR))
    {
        continue;
    }
    else if ((received < 0) &&
             ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK)))
    {
        break;
    }
    else
    {
        /* EOF或真实错误 */
        break;
    }
}
```

### 本次判断错题

错误结论：

> ET模式每次只调用一次`recv()`，剩余数据还会自动再次通知。

正确结论：

> ET模式如果没有读取到`EAGAIN`，缓冲区可能一直保持可读状态，但没有产生新的状态边沿，剩余数据可能长期得不到处理。

---

## 十五、Linux串口`termios`

基础配置：

```c
int serial_fd =
    open(device_path,
         O_RDWR | O_NOCTTY);

struct termios options;

tcgetattr(serial_fd, &options);

cfmakeraw(&options);

cfsetispeed(&options, B115200);
cfsetospeed(&options, B115200);

options.c_cflag &= ~CSIZE;
options.c_cflag |= CS8;
options.c_cflag &= ~PARENB;
options.c_cflag &= ~CSTOPB;
options.c_cflag |= CLOCAL | CREAD;

tcsetattr(serial_fd,
          TCSANOW,
          &options);
```

### 1. Raw模式

`cfmakeraw()`主要关闭：

- 规范输入模式。
- 终端回显。
- 特殊字符处理。
- 换行转换。
- 软件流控等字符处理。

Raw模式使程序读取原始字节。

但是`cfmakeraw()`不会自动完成：

- 115200波特率。
- 8位数据位。
- 无校验。
- 1位停止位。
- 具体的`VMIN/VTIME`策略。

### 2. 115200 8N1

```text
115200：波特率
8：8位数据位，CS8
N：无校验，清除PARENB
1：1位停止位，清除CSTOPB
```

### 3. 常见标志

| 标志 | 含义 |
|---|---|
| `CLOCAL` | 忽略调制解调器控制线 |
| `CREAD` | 开启接收器 |
| `TCSANOW` | 立即应用配置 |
| `O_NOCTTY` | 不让串口成为进程控制终端 |

---

## 十六、`VMIN/VTIME`

`VTIME`的单位是0.1秒，不是毫秒。

| VMIN | VTIME | 行为 |
|---:|---:|---|
| `0` | `0` | 立即返回，没有数据时返回0 |
| `>0` | `0` | 至少收到`VMIN`字节才返回，可能无限阻塞 |
| `0` | `>0` | 从`read()`开始计时，收到首字节或超时后返回 |
| `>0` | `>0` | 无限等待首字节，之后使用字节间超时 |

如果文件描述符设置了：

```c
O_NONBLOCK
```

非阻塞语义可能覆盖`VMIN/VTIME`的阻塞预期，程序必须检查真实返回值和`errno`。

---

## 十七、串口线程如何安全退出

假设：

```text
VMIN = 1
VTIME = 0
```

STM32已连接，但不再发送数据，此时串口线程可能一直阻塞在：

```c
read(serial_fd, buffer, length);
```

其他线程虽然还能运行，但主线程执行：

```c
pthread_join(serial_thread, NULL);
```

可能永远无法完成。

可选方案：

### 方案1：有限超时

```text
VMIN = 0
VTIME > 0
```

让`read()`周期返回，然后检查停止标志。

### 方案2：非阻塞I/O

```text
O_NONBLOCK
+
poll/select/epoll
```

### 方案3：停止事件

使用：

```text
eventfd
或
self-pipe
```

把串口事件和停止事件放入同一个等待集合。

这样停止线程不需要等待STM32恰好再发送一个字节。

---

## 十八、网关有界队列过载

假设：

```text
队列容量：100条
生产速度：50条/秒
消费速度：10条/秒
```

净增长速度：

```text
50 - 10 = 40条/秒
```

填满时间：

```text
100 / 40 = 2.5秒
```

回答“约3秒”方向正确，精确结果是2.5秒。

### 为什么不能使用无界队列

无界队列只会把过载问题推迟为：

- 内存不断增加。
- 排队时延越来越大。
- 旧命令失去实时意义。
- 最终可能发生OOM。
- 恢复后产生大量积压数据突发处理。

### 常见过载策略

- 阻塞生产者，形成背压。
- 拒绝新消息。
- 丢弃最旧消息。
- 丢弃最新消息。
- 合并同类状态。
- 限流。
- 持久化。
- 使用优先级队列。

### 本项目的消息分类

姿态数据：

```text
允许合并
保留最新值
```

紧急停止命令：

```text
高优先级队列
独立通道
预留队列槽位
必要时清除普通动作
```

配置命令：

```text
保证顺序
ACK
超时重试
幂等
不能静默丢弃
```

不能使用一种丢弃策略处理所有消息。

---

## 十九、MQTT回调不能阻塞

如果MQTT回调运行在网络循环线程，回调中执行以下操作可能阻塞整个客户端：

- 等待队列空位。
- 同步写串口。
- 等待STM32响应。
- 执行慢速磁盘日志。
- 进行复杂计算。

可能影响：

- MQTT收包。
- Keep Alive。
- PUBACK处理。
- 断线检测。
- 自动重连。

推荐流程：

```text
基础校验
→ 复制Payload
→ 添加source、sequence、timestamp
→ 非阻塞入队
→ 更新统计信息
→ 尽快返回
```

MQTT回调提供的Payload内存通常只在回调期间有效。异步处理前必须复制到生命周期受控的缓冲区。

过载时应记录：

- 当前队列深度。
- 队列历史高水位。
- 丢弃次数。
- 丢弃原因。
- 消息类型。
- sequence和source。
- 排队时延。
- 处理时延。

如果上游协议支持，应返回：

```text
BUSY
OVERLOAD
```

---

## 二十、Day17判断题复盘

成绩：

```text
13 / 15
```

正确答案：

```text
1×  2√  3√  4×  5√
6×  7×  8√  9×  10√
11× 12× 13√ 14× 15×
```

### 错题1：`EINTR`

错误理解：

> `send()`或`recv()`返回`-1`且`errno == EINTR`说明连接断开。

正确结论：

> `EINTR`表示系统调用被信号中断，不代表连接断开。通常应保持当前偏移并重新调用。

连接错误通常结合以下错误判断：

```text
EPIPE
ECONNRESET
ENOTCONN
```

### 错题2：epoll ET读取

错误理解：

> ET事件触发后读取一次就够了。

正确结论：

> ET模式需要使用非阻塞I/O，并持续读取到`EAGAIN/EWOULDBLOCK`。

---

## 二十一、编程错误总表

| 错误 | 原因 | 正确做法 |
|---|---|---|
| `total_sent++` | 把系统调用次数当成字节数 | `total_sent += sent` |
| `EINTR`分支重复调用并声明变量 | 变量遮蔽、返回值处理不完整 | 使用`continue` |
| `master_set = client_fd` | 混淆集合和描述符 | 使用`FD_SET` |
| `max_fd;` | 没有完成赋值 | `max_fd = client_fd` |
| `FD_CLR(fd, &master_set)` | 参数已经是指针 | `FD_CLR(fd, master_set)` |
| `descriptor == max_fd` | 混淆指针和值 | `descriptor == *max_fd` |
| `*(max_fd)--` | 运算符优先级错误 | `(*max_fd)--` |
| ET只调用一次`recv()` | 混淆LT和ET | 读取到`EAGAIN` |
| 第二条完整帧留到下次 | 没有循环分帧 | 一次处理全部完整帧 |

改进方法：

1. 调用系统函数前先写返回值状态表。
2. 涉及指针时写清楚变量类型。
3. 区分集合对象和集合中的元素。
4. 写循环前明确偏移量和循环不变量。
5. `*`与`++/--`组合时主动加括号。
6. 所有错误路径都检查资源是否需要关闭。

---

## 二十二、面试回答

### 1. 为什么TCP会出现半包和粘包？

> TCP提供可靠有序的字节流，但不保存应用层消息边界，因此一次`send()`不一定对应一次`recv()`。我会为每个连接维护累计缓冲区，把新字节追加进去，再按照分隔符、固定长度或长度字段循环解析所有完整帧，只保留最后的不完整尾部。同时限制最大帧长，避免异常长度导致内存不断增长。

### 2. 为什么`send()`需要循环？

> `send()`返回的是本次实际写入内核发送缓冲区的字节数，不保证等于请求长度。我会维护已发送偏移，下一次从`buffer + offset`继续发送。遇到`EINTR`保持偏移重试；非阻塞Socket遇到`EAGAIN`时保存待发状态并等待写就绪，而不是原地忙等。

### 3. 为什么调用`select()`前要复制集合？

> `select()`会修改传入的`fd_set`，只保留本轮就绪描述符，所以我用`master_set`长期保存全部连接，每轮复制到`read_set`后再调用`select()`。新连接加入`master_set`并更新`max_fd`，断开时删除描述符并重新维护最大值。

### 4. `select()`和`epoll`有什么区别？

> `select()`每轮需要复制集合并扫描0到`max_fd`，还受`FD_SETSIZE`限制，适合连接数量较少的场景。`epoll`将关注事件注册到内核，`epoll_wait()`直接返回就绪事件，更适合Linux高并发。ET模式需要非阻塞I/O并一直处理到`EAGAIN`。

### 5. Linux串口如何配置115200 8N1？

> 我先用`tcgetattr()`读取配置，调用`cfmakeraw()`关闭终端字符处理，再使用`cfsetispeed()`和`cfsetospeed()`设置B115200。清除`CSIZE`后设置`CS8`，清除`PARENB`和`CSTOPB`得到8N1，再设置`CLOCAL | CREAD`。最后根据业务设置`VMIN/VTIME`并调用`tcsetattr()`。Raw模式本身不等于已经完成波特率和8N1配置。

### 6. 网关队列满了怎么办？

> 我不会直接改成无界队列，因为这会导致内存和排队时延不断增长。我会按照消息语义选择策略：姿态数据可以合并并保留最新值；紧急停止命令走高优先级或独立通道；配置命令需要ACK、顺序保证和重试。同时记录队列高水位、丢弃原因和排队时延，并根据上游能力实施背压或限流。

### 7. 为什么MQTT回调中不直接等待队列？

> 如果回调运行在MQTT网络循环线程，阻塞会影响收包、Keep Alive、PUBACK和重连。我会在回调中完成轻量校验、复制Payload和非阻塞入队，然后立即返回。耗时解析、串口控制和日志写入交给工作线程。

### 8. 阻塞串口线程如何安全退出？

> `VMIN=1、VTIME=0`时，设备静默可能让`read()`无限阻塞，导致主线程卡在`pthread_join()`。我会使用有限超时周期检查停止标志，或者使用非阻塞串口配合`poll/epoll`，再通过`eventfd`或self-pipe发送停止事件，使退出不依赖STM32继续发送数据。

---

## 二十三、编译与运行

必须在WSL的仓库根目录执行。

### 1. `SendAll()`测试

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror ./test/day17_send_all.c -o ./test/day17_send_all
./test/day17_send_all
```

预期：

```text
Day17 send_all tests passed
```

### 2. `select()` Echo Server

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror ./test/day17_select_echo_server.c -o ./test/day17_select_echo_server
./test/day17_select_echo_server
```

另开两个WSL终端：

```bash
nc 127.0.0.1 9090
```

分别输入内容，应收到原样回显。

### 编译命令错题

错误：

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \ ./test/day17_send_all.c\ -o ./test/day17_send.all
```

反斜杠后面的空格会被当成参数内容。

反斜杠只有放在行尾时才表示换行续写，而且后面不能存在空格。

最稳妥的写法是使用单行命令：

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror ./test/day17_send_all.c -o ./test/day17_send_all
```

---

## 二十四、当前实现边界

- `day17_send_all.c`使用`socketpair()`进行Linux主机侧本地测试。
- 当前`SendAll()`属于阻塞版实现。
- 非阻塞事件循环还需要发送缓冲区和写就绪状态机。
- `day17_select_echo_server.c`属于多客户端Echo实验，不是完整网关服务。
- Echo Server尚未接入累计缓冲、业务分帧和协议解析。
- `select()`、`epoll`和串口退出方案尚未全部接入真实UART、MQTT和STM32链路。
- 不能将独立实验描述成已经完成的生产级端到端网关。

---

## 二十五、Day17总结

Day17建立了下面这条完整认知链路：

```text
TCP字节流
→ 短写与循环发送
→ 半包粘包与应用层分帧
→ select管理多个连接
→ epoll与LT/ET
→ termios串口阻塞策略
→ 队列过载与安全退出
```

核心原则：

> 系统调用成功不代表业务消息已经完整处理。

> 网络和串口代码必须检查返回值、保存状态、处理边界并设计明确的退出路径。

> 有界队列不是为了保证永不丢数据，而是为了让系统在过载时保持可控。