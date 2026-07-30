# 第五个月 Day14：C++ 第二周复盘与模块集成

## 今日目标

复盘 Day8～Day13 的 C++ 知识，并将 LogManager 与 ProtocolParser 进行最小集成验证。

## 本周完成内容

### Day8：类与对象

- class
- 对象
- public 和 private
- enum class
- 头文件与源文件分离
- g++ 多文件编译

### Day9：构造函数、析构函数与 RAII

- 构造函数
- 析构函数
- 成员初始化列表
- explicit
- std::ofstream
- 文件资源自动释放
- RAII

### Day10：引用与 this 指针

- 值传递
- 普通引用
- const 引用
- this
- *this
- return *this
- 链式调用
- const 成员函数

### Day11：string 与 vector

- std::string
- std::vector
- push_back
- size
- empty
- clear
- erase
- 范围 for
- 最近日志缓存

### Day12：map 与日志统计

- std::map
- key-value
- operator[]
- find
- end
- first 和 second
- 日志等级统计
- const map 引用

### Day13：ProtocolParser

- Command 枚举
- 字符串首尾清洗
- substr
- std::transform
- Lambda
- std::toupper
- map 命令查找
- Unknown 安全状态
- HAND_* 命令映射

## 集成流程

```text
原始字符串
→ ProtocolParser
→ Command 枚举
→ HAND_* 执行命令
→ LogManager 记录处理结果
→ 文件日志、缓存与统计
```

## 模块职责

### ProtocolParser

负责：

- 清洗输入字符串
- 统一大小写
- 识别合法命令
- 转换为 Command 枚举
- 映射为 HAND_* 命令

不负责：

- 串口读取
- MQTT 接收
- 舵机控制
- 文件日志

### LogManager

负责：

- 终端日志
- 文件日志
- 最近日志缓存
- 日志等级统计
- 输出策略配置

不负责：

- 协议解析
- 网络通信
- 设备控制

## 集成实验

测试输入：

```text
OPEN
 grab\r\n
JUMP
STOP
```

结果：

```text
OPEN → HAND_OPEN
GRAB → HAND_GRAB
JUMP → Unknown，记录 WARNING
STOP → HAND_STOP
```

统计结果：

```text
INFO    3
WARNING 1
ERROR   0
```

## C 与 C++ 的项目区别

原有 C 网关主要通过：

```text
结构体
普通函数
模块化 .c/.h
```

新增 C++ 模块主要通过：

```text
类
对象
封装
RAII
STL 容器
const 引用
```

当前没有直接推翻原有 C 网关，而是先通过独立 C++ 模块验证，再逐步集成，降低重构风险。

## 当前不足

1. LogManager 暂未加入时间戳
2. LogManager 暂不支持多线程安全
3. vector 头部删除为 O(n)
4. 日志等级固定时 map 不是最优结构
5. ProtocolParser 当前只支持简单文本命令
6. 尚未集成真实串口、TCP 或 MQTT 输入
7. 尚未实现 ACK 和执行端状态反馈

## 后续优化方向

- 使用 std::chrono 增加时间戳
- 使用 std::mutex 保护日志模块
- 使用环形缓冲区优化最近日志缓存
- 将固定统计改为 std::array
- 增加协议长度与格式检查
- 接入真实串口输入
- 增加 ACK 和错误码

## 今日收获

本周不是单独学习 C++ 语法，而是将类、引用、RAII、string、vector 和 map 用于 Linux IoT Gateway 的真实模块。

当前已经形成：

```text
LogManager
+
ProtocolParser
+
集成 Demo
+
README
+
notes
+
GitHub commit
```

## 下周计划

继续进行 Linux 网关增强，重点学习：

- C++ 多线程基础
- std::thread
- std::mutex
- lock_guard
- 线程安全日志
- 协议模块接入网关主流程