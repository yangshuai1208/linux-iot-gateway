# 第五个月 Day13：ProtocolParser 协议解析类

## 今日目标

使用 C++ 实现 ProtocolParser 协议解析类，将 OPEN、GRAB、RELEASE、STOP 文本命令转换为统一枚举和 STM32 执行命令。

## 今日完成内容

1. 创建独立 ProtocolParser 模块
2. 定义 Command 枚举
3. 使用 std::map 建立命令映射
4. 使用 const string 引用接收输入
5. 去除首尾空格、Tab、回车和换行
6. 使用 substr 截取有效字符串
7. 使用 std::transform 统一转换为大写
8. 使用 Lambda 表达式处理字符
9. 使用 map::find 查询命令
10. 实现合法性检查
11. 实现 HAND_* 执行命令映射
12. 完成正常和异常输入测试
13. 更新 README 和 GitHub

## 协议解析流程

```text
原始字符串
→ 去除首尾空白字符
→ 转换为大写
→ map 查找
→ Command 枚举
→ HAND_* 执行命令
```

示例：

```text
"  grab\r\n"
→ "GRAB"
→ Command::Grab
→ HAND_GRAB
```

## Command 枚举

```cpp
enum class Command
{
    Open,
    Grab,
    Release,
    Stop,
    Unknown
};
```

使用枚举可以避免业务代码反复比较字符串，同时减少命令拼写错误。

Unknown 用于处理非法输入、空字符串和不支持的命令。

## std::map 命令映射

```cpp
std::map<std::string, Command> commandMap_;
```

映射关系：

```text
OPEN    → Command::Open
GRAB    → Command::Grab
RELEASE → Command::Release
STOP    → Command::Stop
```

使用 find() 查询不会创建不存在的键。

## 字符串清洗

使用：

```cpp
find_first_not_of(" \t\r\n")
find_last_not_of(" \t\r\n")
substr()
```

去除串口输入中常见的空格、Tab、回车和换行。

当字符串全部为空白字符时，find_first_not_of() 返回：

```cpp
std::string::npos
```

此时直接返回空字符串。

## std::transform

```cpp
std::transform(
    text.begin(),
    text.end(),
    text.begin(),
    [](unsigned char character)
    {
        return static_cast<char>(
            std::toupper(character));
    });
```

作用是将整个字符串统一转换为大写。

这样 OPEN、open、Open 都可以识别为同一个命令。

## Lambda 表达式

```cpp
[](unsigned char character)
{
    return static_cast<char>(
        std::toupper(character));
}
```

Lambda 是一个临时匿名函数。

本项目中用于逐个处理字符串中的字符。

## find 与 end

```cpp
const auto iterator =
    commandMap_.find(normalizedInput);
```

找到命令：

```cpp
iterator != commandMap_.end()
```

未找到：

```cpp
iterator == commandMap_.end()
```

未找到时返回 Command::Unknown。

## 执行端映射

```text
Command::Open    → HAND_OPEN
Command::Grab    → HAND_GRAB
Command::Release → HAND_RELEASE
Command::Stop    → HAND_STOP
Command::Unknown → HAND_NONE
```

HAND_NONE 可以避免非法命令直接驱动执行机构。

## 模块边界

ProtocolParser 只负责：

- 清洗输入
- 识别命令
- 转换枚举
- 输出统一执行命令

它不负责：

- 串口读取
- MQTT 接收
- 舵机控制
- 文件日志
- 网络转发

保持单一职责便于后续测试和替换协议。

## 实验结果

正常命令可以正确解析。

小写、混合大小写和带空格命令也能识别。

JUMP 和空字符串会返回：

```text
Command::Unknown
HAND_NONE
valid=false
```

## 今日收获

协议解析前应先统一输入格式。

业务层不应长期依赖原始字符串，应尽早转换成枚举或结构化数据。

非法命令必须进入安全默认状态，不能直接控制执行器。

## 明日计划

完成第二周复盘，整理 Day8～Day13 的 C++ 技术栈、LogManager、ProtocolParser、README、简历素材和面试题。