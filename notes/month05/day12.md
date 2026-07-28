# 第五个月 Day12：std::map 与日志等级统计

## 今日目标

学习 std::map、键值对和查找接口，并为 LogManager 增加按日志等级统计数量的功能。

## 今日完成内容

1. 学习 std::map
2. 理解 key-value 键值对
3. 学习 operator[]
4. 学习 find()
5. 学习 end()
6. 学习 first 和 second
7. 使用范围 for 遍历 map
8. 使用 const 引用返回 map
9. 实现日志等级数量统计
10. 实现单个等级查询
11. 实现全部统计查询
12. 实现统计清零
13. 更新 README
14. 完成 GitHub 提交

## std::map

std::map 是键值对容器。

```cpp
std::map<LogLevel, std::size_t> logCounts_;
```

其中：

```text
LogLevel
→ key，日志等级

std::size_t
→ value，日志数量
```

示例数据：

```text
Info    → 3
Warning → 1
Error   → 1
```

## operator[]

使用：

```cpp
++logCounts_[level];
```

如果键已经存在，则返回对应的值。

如果键不存在，operator[] 会创建新键，并将数值类型默认初始化为 0，然后再进行加一。

operator[] 可能修改 map，因此不能在 const 查询函数中使用它查询不存在的键。

## find()

查询时使用：

```cpp
auto iterator = logCounts_.find(level);
```

找到时：

```cpp
iterator != logCounts_.end()
```

未找到时：

```cpp
iterator == logCounts_.end()
```

读取 value：

```cpp
iterator->second
```

## first 与 second

map 中的每个元素都是键值对。

```cpp
entry.first
```

表示键。

```cpp
entry.second
```

表示值。

本项目中：

```text
entry.first
→ LogLevel

entry.second
→ 日志数量
```

## 遍历 map

```cpp
for (const auto &entry : logCounts)
{
    std::cout << entry.second << '\n';
}
```

使用 const 引用可以避免复制键值对，并禁止遍历时修改元素。

## 返回 map 的 const 引用

```cpp
const std::map<LogLevel, std::size_t> &
getAllLogCounts() const;
```

返回引用避免复制整个 map。

返回 const 引用可以防止调用者直接修改内部统计数据。

成员函数末尾的 const 表示该查询函数不会修改 LogManager 对象。

## 统计清零

```cpp
for (auto &entry : logCounts_)
{
    entry.second = 0;
}
```

map 的键不能通过迭代器直接修改，但 value 可以修改。

resetLogCounts() 只清零统计，不删除日志等级键。

## map 的特点

std::map 通常基于平衡树实现。

特点：

- 键保持有序
- 键不能重复
- 查找、插入和删除通常为 O(log n)
- 元素不是连续内存存储
- 每个节点存在额外内存开销

## map 与 unordered_map

std::map：

- 键有序
- 通常使用平衡树
- 查找复杂度 O(log n)

std::unordered_map：

- 键通常无序
- 通常使用哈希表
- 平均查找复杂度 O(1)

当前日志等级只有三个，性能差异可以忽略。

## 嵌入式使用注意

Linux 网关程序中可以使用 std::map。

资源受限 MCU 中需要谨慎，因为 map：

- 使用动态内存
- 每个节点有额外开销
- 内存不连续
- 操作时间和内存分配不适合严格实时场景

日志等级固定时，MCU 中更适合使用：

```cpp
std::array<std::size_t, 3>
```

或固定长度 C 数组。

## 实验结果

共生成：

```text
INFO    3 条
WARNING 1 条
ERROR   1 条
```

调用 resetLogCounts() 后，三个等级数量全部变成 0。

日志文件和最近日志缓存不受统计清零影响。

## 今日收获

std::map 适合表达键和值之间的映射关系。

operator[] 适合插入或更新数据，find() 适合只读查询和判断键是否存在。

## 明日计划

将当前 C++ 基础整合为 ProtocolParser 类，解析网关命令字符串，并将 OPEN、GRAB、RELEASE、STOP 映射为统一命令枚举。