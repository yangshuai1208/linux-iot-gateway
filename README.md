# linux-iot-gateway

## 项目简介

本项目是一个基于 Linux C 的物联网网关系统，主要用于接收下位机 STM32 通过串口发送的传感器数据，并将数据转发到 TCP 客户端或 MQTT Broker，同时通过日志模块记录系统运行状态。

项目目标不是简单串口打印，而是模拟真实物联网系统中的边缘网关：负责设备接入、协议转换、数据转发、异常记录和后续系统扩展。

## 项目功能

- 串口接收 STM32 下位机数据
- TCP Server 向客户端转发数据
- MQTT Client 上传传感器数据
- 日志模块记录系统运行状态
- Makefile 一键编译
- 支持模块化代码结构
- 为后续 AIoT 智能眼镜 + 灵动手控制系统提供网关基础

## 系统架构

```mermaid
flowchart TD
    A[STM32 下位机] -->|UART 串口| B[Linux IoT Gateway]

    B --> C[serial 模块]
    B --> D[tcp_server 模块]
    B --> E[mqtt_client 模块]
    B --> F[log 模块]

    C --> G[解析串口数据]
    D --> H[TCP 客户端]
    E --> I[MQTT Broker]
    F --> J[本地日志文件]

    I --> K[云端 / 上位机 / 其他设备]
    
---


```markdown
## 编译与运行

详细说明见：

- [编译与运行说明](docs/build_run.md)
- [测试记录](docs/test_record.md)

基础编译：

```bash
make
### LogManager 构造函数与析构函数

Day9 为 LogManager 增加了文件资源管理功能。

对象生命周期：

```text
创建 LogManager 对象
→ 构造函数打开日志文件
→ log() 输出终端与文件日志
→ 对象离开作用域
→ 析构函数刷新并关闭日志文件
```

当前支持：

- `INFO`、`WARNING`、`ERROR` 日志等级
- 终端日志输出
- 文件日志输出
- 追加模式保存日志
- 构造函数自动打开文件
- 析构函数自动关闭文件
- `isFileOpen()` 检查文件状态

运行：

```bash
cd cpp_modules/log_manager
make
make run
cat log_manager_demo.log
```

该模块通过 RAII 将日志文件的打开和关闭与对象生命周期绑定，降低忘记释放文件资源的风险。

### 引用、this 指针与链式配置

Day10 为 LogManager 增加了运行参数配置功能：

- 使用 `const std::string &` 避免日志消息复制
- 使用 `this->` 明确访问当前对象成员
- 使用 `return *this` 返回当前对象引用
- 支持配置接口链式调用
- 支持启用或关闭终端日志
- 支持启用或关闭自动刷新
- 支持只读查询日志文件路径

链式配置示例：

```cpp
LogManager logger("gateway.log");

logger.setConsoleEnabled(true)
      .setAutoFlush(true);
```

当前模块已经覆盖：

```text
类与对象
→ 构造函数和析构函数
→ RAII 文件资源管理
→ 引用传参
→ this 指针
→ 链式调用
```
### 最近日志缓存

Day11 使用 `std::vector<std::string>` 为 LogManager 增加了最近日志缓存功能。

当前支持：

- 使用 `std::string` 构造日志文本
- 使用 `std::vector` 保存最近日志
- 使用 `push_back()` 添加日志
- 使用 `size()` 和 `empty()` 查询缓存状态
- 使用范围 `for` 循环遍历日志
- 使用 `const` 引用避免容器复制
- 支持配置最大缓存数量
- 超过容量后自动删除最旧日志
- 支持清空内存缓存

示例：

```cpp
LogManager logger("gateway.log");

logger.setMaxCachedLogs(3);

const std::vector<std::string> &logs =
    logger.getRecentLogs();

for (const std::string &line : logs)
{
    std::cout << line << '\n';
}
```

当前缓存使用 `vector::erase(begin())` 删除最旧元素，适合少量日志演示。高频或大容量场景后续可改为环形缓冲区。

### 日志等级统计

Day12 使用 `std::map<LogLevel, std::size_t>` 为 LogManager 增加了日志等级统计功能。

当前支持：

- 按 `INFO`、`WARNING`、`ERROR` 统计日志数量
- 使用 `operator[]` 更新统计值
- 使用 `find()` 查询指定等级
- 使用范围 for 遍历所有统计结果
- 使用 `const map&` 避免复制全部数据
- 支持统计数据清零

示例：

```cpp
logger.log(LogLevel::Info, "Gateway started");
logger.log(LogLevel::Error, "Parse failed");

std::size_t errorCount =
    logger.getLogCount(LogLevel::Error);
```

当前使用 `std::map` 主要用于学习键值映射。在日志等级固定、性能要求更高的场景中，可以使用 `std::array` 或固定数组优化内存和访问效率。

### ProtocolParser 协议解析模块

Day13 新增独立的 C++ 文本协议解析模块：

```text
cpp_modules/protocol_parser/
```

当前支持：

- `OPEN`
- `GRAB`
- `RELEASE`
- `STOP`
- 非法命令识别
- 首尾空格、Tab、回车和换行清洗
- 大小写统一
- 文本命令转 `Command` 枚举
- `Command` 枚举转 `HAND_*` 执行命令

处理流程：

```text
原始文本
→ 输入清洗
→ 大写转换
→ map 查找
→ Command 枚举
→ HAND_* 命令
```

示例：

```text
" grab\r\n"
→ Command::Grab
→ HAND_GRAB
```

运行：

```bash
cd cpp_modules/protocol_parser
make
make run
```

未知命令统一转换为 `Command::Unknown` 和 `HAND_NONE`，避免非法输入直接驱动执行端。

### C++ 多线程基础 Demo

Day15 新增：

```text
cpp_modules/thread_demo/
```

该模块使用 `std::thread` 创建采集线程和处理线程：

```text
采集线程
→ 模拟生成传感器数据
→ 原子状态通知

处理线程
→ 检查新数据
→ 处理传感器值

主线程
→ join 等待线程结束
→ 安全退出
```

当前涉及：

- `std::thread`
- `join()`
- `joinable()`
- 线程 ID
- `std::this_thread::sleep_for()`
- `std::atomic`
- `load()`、`store()`、`exchange()`
- `-pthread` 编译参数

运行方法：

```bash
cd cpp_modules/thread_demo
make
make run
```

当前使用单槽共享数据和轮询，仅用于线程基础验证。后续将使用 `std::mutex`、`std::condition_variable` 和队列实现更完整的线程通信。

现有 LogManager 暂未声明为线程安全模块，不能直接由多个线程同时调用。