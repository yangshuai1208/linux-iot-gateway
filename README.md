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

### 线程安全 LogManager

Day16 为 `LogManager` 增加了线程同步保护。

涉及：

- `std::mutex`
- `std::lock_guard`
- `mutable mutex`
- 临界区
- 数据竞争
- 线程安全查询
- `std::ref`
- 多线程日志测试

并发测试流程：

```text
collector 线程 ─┐
                ├→ 共享 LogManager
processor 线程 ─┘
```

一次日志操作会在同一临界区内完成：

```text
终端输出
→ 文件写入
→ 最近日志缓存
→ 等级统计
```

为了避免容器内部引用在锁释放后逃逸，以下接口改为返回副本：

```cpp
std::vector<std::string> getRecentLogs() const;
std::map<LogLevel, std::size_t>
getAllLogCounts() const;
```

运行：

```bash
cd cpp_modules/thread_safe_log_demo
make clean
make
make run
```

预期统计：

```text
INFO=5
WARNING=5
ERROR=0
cached_logs=10
```

当前要求所有使用 LogManager 的工作线程在对象析构前完成 join。

### C++ 生产者消费者模型

Day17 新增：

```text
cpp_modules/producer_consumer_demo/
```

使用：

- `std::queue`
- `std::mutex`
- `std::unique_lock`
- `std::condition_variable`
- `wait()`
- `notify_one()`
- `notify_all()`

数据流：

```text
采集线程
→ SensorSample
→ 线程安全 queue
→ condition_variable 通知
→ 数据处理线程
```

与早期 atomic 单槽轮询方案相比：

```text
轮询 + sleep
```

升级为：

```text
阻塞等待 + 主动唤醒
```

运行：

```bash
cd cpp_modules/producer_consumer_demo
make
make run
```

当前 Demo 使用单生产者、单消费者结构，后续计划接入 Linux IoT Gateway 的真实协议处理流程。

### C++ 多线程网关最小闭环

Day18 将此前的 C++ 模块进行最小业务集成：

```text
模拟 UART/MQTT 输入线程
        ↓
线程安全命令队列
        ↓
condition_variable
        ↓
协议处理线程
        ↓
ProtocolParser
        ↓
Command / HAND_*
        ↓
线程安全 LogManager
```

涉及：

- `std::thread`
- `std::queue`
- `std::mutex`
- `std::lock_guard`
- `std::unique_lock`
- `std::condition_variable`
- `std::ref`
- `ProtocolParser`
- 线程安全 `LogManager`

测试：

```text
OPEN       → HAND_OPEN
GRAB       → HAND_GRAB
JUMP       → Reject
STOP       → HAND_STOP
RELEASE    → HAND_RELEASE
```

运行：

```bash
cd cpp_modules/gateway_pipeline_demo
make clean
make
make run
```

当前属于模拟输入下的 C++ 并发网关验证模块，尚未正式接入真实 UART、TCP 或 MQTT 数据源。

### 结构化命令与 ACK Demo

Day19 在多线程命令处理闭环基础上增加结构化请求和状态反馈。

```text
RawCommand
├── sequence
├── source
└── payload
        ↓
ProtocolParser
        ↓
Command / HAND_*
        ↓
CommandResult
        ↓
ACK / ERROR
```

示例：

```text
seq=1 OPEN
→ ACK seq=1 code=0 HAND_OPEN

seq=3 JUMP
→ ERROR seq=3 code=1001 INVALID_COMMAND
```

当前 ACK 表示 Linux 网关侧完成协议解析与指令映射，并非 STM32 实际执行反馈。

运行：

```bash
cd cpp_modules/gateway_ack_demo
make clean
make
make run


### ACK 超时与重试

Day20 增加 ACK 超时检测和简单重试：

```text
Command
→ sequence
→ send
→ wait_for ACK
→ success

timeout
→ retry
→ ACK / FAILED

### 二进制串口协议实验

Day21 实现二进制协议帧编码、解码和流式解析：

```text
AA 55
LEN
SEQ_H SEQ_L
CMD
CHECKSUM
0D 0A

### Sequence 去重与重复请求处理

Day22 针对 ACK 丢失后的重传问题增加请求去重机制。

```text
Request seq=3
→ execute
→ cache ACK
→ ACK lost

Retry seq=3
→ duplicate detected
→ skip execution
→ resend cached ACK
## C++17 网关增强实验

为验证 Linux IoT Gateway 后续 C++ 化方案，增加了一组独立实验模块。

### 并发处理

```text
Input Producer
→ thread-safe queue
→ condition_variable
→ Command Consumer
→ ProtocolParser
→ LogManager

### 当前实现边界

原始 Linux IoT Gateway 主链路已实现数据源抽象、协议解析、JSON 转换、日志记录和 MQTT 发布。

C++17 目录主要用于后续架构增强实验，包括：

- ProtocolParser
- Thread-safe LogManager
- Producer / Consumer
- ACK / sequence
- timeout / retry
- binary frame
- duplicate detection

其中可靠通信及二进制协议模块目前主要在 Linux 模拟环境完成验证，尚未全部集成到真实 UART + STM32 链路。

### C++ RAII 与资源所有权实验

Day25 增加智能指针与 RAII 实验，结合网关资源管理理解：

- `std::unique_ptr`
- `std::shared_ptr`
- `std::weak_ptr`
- `std::make_unique`
- `std::make_shared`
- `std::move`
- RAII
- 资源所有权

实验中使用 `unique_ptr` 表示串口对象唯一所有权，
使用 `shared_ptr` 表示共享配置生命周期，
并通过 `weak_ptr` 实现非拥有型观察。

该模块用于 C++17 资源管理知识验证。


---

```markdown
### C++ 移动语义实验

增加 `move_semantics_demo`，用于验证现代 C++ 中的：

- 左值 / 右值
- 右值引用
- `std::move`
- 拷贝构造
- 移动构造
- 拷贝赋值
- 移动赋值
- Rule of Five / Rule of Zero

实验通过自定义 Buffer 资源类，对比深拷贝和资源所有权转移过程。

实际网关代码仍优先使用 STL、智能指针及 RAII 类型管理资源。

### Lambda 与 STL Algorithm 实验

增加 `lambda_algorithm_demo`，用于复习：

- Lambda 表达式
- 值捕获 / 引用捕获
- Predicate
- `std::for_each`
- `std::transform`
- `std::find_if`
- `std::count_if`

实验模拟网关命令记录，对命令进行标准化、查找和状态统计。

这些知识同时应用于：

- ProtocolParser 命令标准化
- condition_variable 条件谓词
- 网关状态筛选与统计

1. log_manager
线程安全日志

2. protocol_parser
命令解析

3. gateway_pipeline_demo
Producer / Consumer

4. gateway_timeout_demo
ACK + timeout + retry

5. reliable_serial_demo
sequence dedup + cached ACK


## 二、更新`linux-iot-gateway/README.md`

追加：

```markdown
### 第六阶段 Day 3：Linux Gateway与串口数据源复习

本次复习和改进内容：

- 复盘ESP32-S3、Linux Gateway、STM32F407三端职责
- 修复传感器JSON格式和缓冲区截断问题
- 增加数据解析和JSON构造单元测试
- 补充Linux串口`open/read/close/termios`逻辑
- 复习115200、8N1、`VMIN/VTIME`
- 复习MQTT QoS、消息去重和断线缓存设计
- 分析串口半包、粘包、CRC和状态机方案

验证结果：

```text
sensor parser tests passed

```markdown
### Day 10：TCP分帧与MQTT可靠性

- 复习TCP字节流、粘包与拆包问题。
- 复习`send()`、`recv()`返回值和非阻塞错误处理。
- 实现长度字段二进制协议解析状态机。
- 支持帧头同步、长度检查、Payload接收和异或校验。
- 增加错误长度、错误校验、连续帧和噪声恢复测试。
- 审查Linux网关MQTT模块的QoS、网络循环和断线恢复问题。
- 总结有界离线队列、指数退避、随机抖动和消息幂等设计。

验证结果：

```text
length-prefixed framer tests passed

## Day14：PCA9685舵机映射单元测试

新增`test/day14_servo_angle_to_count.c`，验证舵机角度到PCA9685 PWM计数值的映射逻辑。

- 使用50Hz、20ms周期和12位PWM分辨率。
- 覆盖0°、90°、180°及非法参数测试。
- 参数错误时保证不修改输出参数。
- 测试已通过：`Servo angle conversion tests passed`。

> 该文件是可复用的控制算法单元测试，用于验证PCA9685计数计算；不代表Linux网关当前直接驱动舵机硬件。


今天 15:15

8a73e147-22b3-40ec-add4-d08e33c34553.jpg

7db7dd8e-8ca2-4f4d-80e5-c984da0e1c86.jpg
给出题目和解析，解析可以稍微短一点

好，“有效括号”算法题已经完成并通过测试。Day16现在只剩三个收尾部分：

Linux网关项目结合。

当天岗位投递。

更新notes、README并完成第二次Git提交。

项目结合题
请按面试方式回答：

如果把今天实现的pthread生产者消费者队列接入Linux IoT Gateway，你会怎样划分生产者和消费者？队列里保存什么数据？为什么队列应该设计成有界队列？程序退出时怎样保证所有线程安全结束？

按五个关键词回答：

生产者
→ RawCommand队列
→ 协议消费者
→ 背压
→ finished + broadcast + join
注意项目边界：

目前可以说是在独立实验中验证了生产者消费者模型，以及说明计划如何接入网关；如果尚未接入真实UART/MQTT主链路，不能说已经完成实际集成。

请控制在60～90秒。柏。


9d9a8232-d768-4648-8b9f-8de7d13c5463.jpg

这段回答抓住了“原始命令、有限队列、广播退出”三个点，但生产者与消费者划分不准确，退出流程也不完整，给你 65分。

一、生产者和消费者划分
在Linux网关中，更合理的划分是：

MQTT接收线程
UART/TCP接收线程
→ 生产者
它们把收到的命令放入队列。

协议解析与分发线程
→ 消费者
消费者负责：

校验命令。

解析协议。

映射为HAND_OPEN等执行命令。

交给串口发送模块。

日志线程可以拥有自己的日志队列，但不应直接作为原始命令队列的主要消费者，否则可能与协议线程争抢并取走命令。

二、队列中保存什么
不建议只保存一段没有边界信息的字符串，可以保存结构化命令：

typedef struct
{
    unsigned int sequence;
    CommandSource source;
    size_t length;
    char payload[64];
} RawCommand;
这样可以保留：

来源
sequence
有效长度
命令内容
三、为什么使用有界队列
你说的“无界队列可能耗尽内存”是正确的。

如果生产速度长期高于消费速度：

消息不断进入
→ 队列不断增长
→ 内存持续占用
→ 最终分配失败或进程崩溃
有界队列能形成背压。队列满时可以：

阻塞普通生产者。

返回繁忙状态。

记录告警。

根据业务决定拒绝新命令。

控制命令不能在没有记录的情况下随意丢弃。

四、安全退出流程
仅仅broadcast()不够，必须先修改退出条件：

停止接收新命令
→ 等待生产者线程结束
→ 加锁
→ finished = true
→ broadcast唤醒消费者
→ 解锁
→ 消费者处理完剩余队列
→ 消费者退出
→ pthread_join回收线程
→ 销毁mutex和condition
如果先把消费者关闭，但生产者还在继续写队列，可能产生数据丢失或访问已销毁资源的问题。

面试推荐版本
如果接入Linux网关，我会把MQTT、UART或TCP接收线程作为生产者，将收到的消息封装成包含source、sequence、length和payload的RawCommand后放入有界线程安全队列。协议处理线程作为消费者，完成校验、解析和执行命令映射。使用有界队列是为了在生产速度超过消费速度时形成背压，避免内存无限增长。退出时先停止并回收生产者，再在互斥锁保护下设置finished并广播唤醒消费者；消费者处理完剩余数据后退出，主线程最后join所有线程并销毁同步资源。目前该模型主要在独立实验中完成验证，尚未全部接入真实网关主链路。

至此，Day16技术学习已经完成：

pthread生产者消费者代码

死锁、条件变量、优先级反转

join、detach、竞态与atomic

12道判断题

有效括号算法

Linux网关线程模型分析

现在只剩：

投递5～10个岗位。

整理Day16错题与知识点。

更新README和notes。

提交有效括号代码及最终Day16 Git记录。


接下来更新并提交GitHub

Day16 的更新材料已经整理完成：

Day16完整notes

README追加内容

将第一份保存为：

notes/day16_linux_pthread_and_stack_review.md
将第二份内容追加到根目录 README.md。如果README中已经存在Day16标题，应更新原段落，不要重复添加。

同时在 .gitignore 中加入：

/test/day16_pthread_producer_consumer
/test/day16_valid_brackets
然后在当前WSL终端执行：

cd ~/linux-iot-gateway

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -pthread \
    ./test/day16_pthread_producer_consumer.c \
    -o /tmp/day16_pthread_producer_consumer

/tmp/day16_pthread_producer_consumer

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
    ./test/day16_valid_brackets.c \
    -o /tmp/day16_valid_brackets

/tmp/day16_valid_brackets
两项测试通过后，精确暂存Day16文件：

git status --short

git add test/day16_pthread_producer_consumer.c
git add test/day16_valid_brackets.c
git add notes/day16_linux_pthread_and_stack_review.md
git add README.md
git add .gitignore

git diff --cached --check
git diff --cached --name-status
确认暂存区中只有Day16相关文件，再提交：

git commit -m "test: 完成Day16 pthread与有效括号复习"
git push origin main
git status --short
不要使用git add .，因为截图中还有Day10文件、Makefile和编译产物等其他改动。如果生产者消费者文件此前已经提交，本次暂存时它没有变化属于正常现象。

推送成功后，Day16技术任务全部完成；如果当天的5～10份定向投递也已完成，Day16即可正式结束。


day16_linux_pthread_and_stack_review.md
文档


day16_README_append.md
文档


资料库
/
day16_README_append.md


第六阶段 Day16：pthread生产者消费者与线程同步复习
新增Linux主机侧实验：

test/day16_pthread_producer_consumer.c
test/day16_valid_brackets.c
本次完成：

使用pthread_create()和pthread_join()创建并回收生产者、消费者线程。

使用固定数组实现容量为4的有界环形队列。

使用pthread_mutex_t保护data、head、tail、count和finished等共享状态。

使用not_empty和not_full条件变量实现阻塞等待与主动唤醒。

使用while重新检查等待谓词，处理虚假唤醒和多线程竞争。

使用finished状态和broadcast实现消费者排空队列后安全退出。

复习死锁四个必要条件、统一加锁顺序、GDB线程栈排查和优先级反转。

复习数据竞争，以及volatile、互斥锁和C11原子操作的边界。

使用固定数组栈实现“有效括号”，覆盖匹配失败、栈下溢、未闭合、非法字符、空指针和栈溢出测试。

编译与运行：

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -pthread \
    ./test/day16_pthread_producer_consumer.c \
    -o ./test/day16_pthread_producer_consumer

./test/day16_pthread_producer_consumer

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
    ./test/day16_valid_brackets.c \
    -o ./test/day16_valid_brackets

./test/day16_valid_brackets
验证结果：

Day16 pthread producer-consumer tests passed
Day16 valid brackets tests passed
实现边界：以上代码属于Linux主机侧独立并发与算法实验，用于验证pthread、有界队列、线程同步和数组栈。相关并发结构尚未全部接入真实UART、TCP、MQTT与STM32主链路，不能描述为已经完成的硬件端到端多线程网关。

### 第六阶段 Day17：Socket、I/O多路复用与串口阻塞复习

新增Linux主机侧实验：

```text
test/day17_send_all.c
test/day17_select_echo_server.c
```

本次完成：

- 复习TCP可靠字节流与UDP数据报的区别。
- 实现阻塞版`SendAll()`，处理短写、发送偏移、`EINTR`和对端关闭。
- 使用`MSG_NOSIGNAL`避免向关闭连接发送时由`SIGPIPE`终止进程。
- 复习`recv()`的正数、0、`EINTR`和`EAGAIN/EWOULDBLOCK`语义。
- 复习TCP半包、粘包，以及累计缓冲区和循环分帧方法。
- 使用`select()`实现单线程多客户端Echo Server。
- 掌握`master_set`、`read_set`、`FD_SET`、`FD_ISSET`、`FD_CLR`和`max_fd`维护。
- 对比`select()`与`epoll`，复习LT和ET触发模式。
- 复习Linux串口Raw模式、115200 8N1和四组`VMIN/VTIME`行为。
- 分析有界队列过载、MQTT回调阻塞和串口线程安全退出方案。

编译与运行：

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror ./test/day17_send_all.c -o ./test/day17_send_all
./test/day17_send_all

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror ./test/day17_select_echo_server.c -o ./test/day17_select_echo_server
./test/day17_select_echo_server
```

另开WSL终端连接Echo Server：

```bash
nc 127.0.0.1 9090
```

验证结果：

```text
Day17 send_all tests passed
select Echo Server多客户端收发测试通过
```

> 实现边界：以上代码属于Linux主机侧Socket与I/O多路复用实验。`SendAll()`当前为阻塞版，Echo Server尚未加入业务分帧、发送缓存和非阻塞写状态机；相关结构尚未全部接入真实UART、MQTT与STM32端到端链路。

## 第六阶段 Day19：C语言高级综合

本日围绕嵌入式开发中的结构体、函数指针、回调函数、对象生命周期和未定义行为进行了综合训练。

### 完成内容

- 复习结构体内存对齐、成员偏移和填充字节。
- 区分函数指针、返回指针的函数和回调函数。
- 使用命令分发表实现`OPEN`和`STOP`命令分发。
- 通过结构体值复制解决回调参数生命周期问题。
- 复习数组退化和尾后指针。
- 使用三次翻转实现数组原地右旋。
- 分析MQTT回调阻塞、Payload所有权和队列过载问题。
- 区分MQTT PUBACK与STM32业务ACK。

### 新增文件

```text
notes/day19_c_advanced_review.md
test/day19_command_dispatch.c
test/day19_rotate_array.c
```

### 数组右旋算法

```text
原数组：       1 2 3 4 5
右移2位：      4 5 1 2 3
时间复杂度：   O(n)
额外空间：     O(1)
```

### 项目应用

MQTT回调只负责消息校验、数据复制和快速入队；串口发送、ACK等待和超时重传应交给独立工作线程处理。

MQTT QoS 1的PUBACK只能确认MQTT链路中的消息接收，不能证明STM32已经执行动作。端到端可靠控制还需要业务ACK、sequence、超时重传和重复请求去重。

> 当前练习用于验证Linux侧命令分发与可靠通信设计，不能将尚未完成的STM32实机ACK闭环描述为已实现功能。
