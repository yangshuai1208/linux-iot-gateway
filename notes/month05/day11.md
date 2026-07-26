# 第五个月 Day11：std::string 与 std::vector

## 今日目标

学习 std::string、std::vector 和范围 for 循环，并为 LogManager 增加最近日志缓存。

## 今日完成内容

1. 学习 std::string 基础
2. 学习 std::vector 动态数组
3. 学习 push_back()
4. 学习 size()
5. 学习 empty()
6. 学习 clear()
7. 学习 erase()
8. 学习 begin()
9. 学习范围 for 循环
10. 使用 const 引用遍历元素
11. 实现最近日志缓存
12. 实现缓存容量限制
13. 实现旧日志自动淘汰
14. 完成 README 和 GitHub 提交

## std::string

std::string 是 C++ 标准库提供的字符串类。

```cpp
std::string message = "gateway started";
```

常用操作：

```cpp
message.size();
message.empty();
message.clear();
```

当前 LogManager 使用字符串拼接生成完整日志：

```cpp
const std::string line =
    "[" + levelToString(level) + "] " + message;
```

## std::vector

std::vector 是动态数组，可以按照运行时需求增加或删除元素。

```cpp
std::vector<std::string> recentLogs_;
```

表示容器中的每个元素都是 std::string。

添加元素：

```cpp
recentLogs_.push_back(line);
```

查询数量：

```cpp
recentLogs_.size();
```

判断为空：

```cpp
recentLogs_.empty();
```

清空：

```cpp
recentLogs_.clear();
```

## 固定容量日志缓存

缓存容量设置为 3 时，只保留最后 3 条日志。

```cpp
recentLogs_.push_back(line);

while (recentLogs_.size() > maxCachedLogs_)
{
    recentLogs_.erase(recentLogs_.begin());
}
```

begin() 指向 vector 的第一个元素，也就是最旧日志。

erase(begin()) 删除最旧日志。

## 范围 for 循环

```cpp
for (const std::string &line : recentLogs)
{
    std::cout << line << '\n';
}
```

const 引用可以避免复制每一个字符串，同时禁止循环内部修改容器元素。

## 返回 vector 的 const 引用

```cpp
const std::vector<std::string> &getRecentLogs() const;
```

第一个 const 表示调用者不能修改返回的 vector。

引用避免复制整个容器。

函数末尾的 const 表示查询函数不会修改 LogManager 对象。

## vector 的内存特点

vector 中的元素通常连续存放。

优点：

- 支持下标访问
- 遍历效率较高
- 使用方便
- 可以动态扩容

注意：

- 动态扩容可能重新申请内存
- 扩容后原有迭代器和指针可能失效
- 在资源受限的 MCU 中需要谨慎使用动态容器

## erase(begin()) 的性能

删除 vector 第一个元素后，后面的元素需要向前移动。

因此：

```cpp
erase(begin())
```

时间复杂度通常为 O(n)。

当前缓存数量很少，可以接受。

大型高频日志系统更适合使用：

- std::deque
- 环形缓冲区
- 固定长度数组

## 实验结果

写入 5 条日志，将缓存容量设置为 3。

内存缓存最终只保留最后 3 条日志。

日志文件仍保存全部日志。

clearRecentLogs() 执行后，缓存数量变为 0。

## 今日收获

std::vector 可以用于管理数量动态变化的一组同类型对象。

通过 const 引用返回容器，可以避免复制并保护内部数据。

范围 for 循环能够更直观地遍历容器元素。

## 明日计划

学习 std::map，使用字符串和日志等级建立映射，并为 LogManager 增加按等级统计日志数量的功能。