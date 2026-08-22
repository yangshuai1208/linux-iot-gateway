# Day 4：TCP/IP与Socket基础

## 今日完成

- 复习TCP/IP四层模型。
- 理解TCP与UDP的主要区别。
- 学习Socket服务端基本流程。
- 实现单客户端阻塞式TCP回显服务器。
- 使用严格警告完成编译。
- 订正Socket预习题和代码错误。

## TCP/IP项目映射

- 应用层：MQTT、JSON、设备命令。
- 传输层：TCP。
- 网络层：IPv4。
- 网络接口层：Wi-Fi或以太网。
- 下位机通信：UART。

MQTT属于应用层，通常基于TCP。

## TCP与UDP

TCP具有面向连接、可靠传输、保证顺序和字节流等特点。

UDP无连接、尽力交付、按数据报传输，开销更低，但不保证可靠性和顺序。

## Socket服务端流程

1. `socket()`创建监听套接字。
2. `setsockopt()`设置`SO_REUSEADDR`。
3. `bind()`绑定本地IP和端口。
4. `listen()`建立连接等待队列。
5. `accept()`接收客户端并返回通信套接字。
6. `recv()`接收数据。
7. `send()`发送数据。
8. `close()`释放文件描述符。

## 两种文件描述符

- `server_fd`：负责监听连接。
- `client_fd`：负责与客户端收发数据。

`accept()`会返回新的`client_fd`，不会改变`server_fd`。

## recv返回值

- 大于0：实际收到的字节数。
- 等于0：TCP对端正常关闭连接。
- 小于0：接收失败，需要检查`errno`。

`recv()`不会自动补字符串结束符，因此读取时需要预留一个字节：

```c
ssize_t received = recv(
    client_fd,
    buffer,
    sizeof(buffer) - 1U,
    0
);

if (received > 0)
{
    buffer[received] = '\0';
}