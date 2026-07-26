# 第五个月 Day10：引用、this 指针与链式调用

## 今日目标

学习 C++ 引用、const 引用和 this 指针，并为 LogManager 增加链式配置接口。

## 今日完成内容

1. 学习值传递
2. 学习普通引用传递
3. 学习 const 引用传递
4. 理解 this 指针
5. 理解 *this
6. 使用 this->访问成员变量
7. 使用 return *this 返回当前对象
8. 实现配置接口链式调用
9. 增加终端输出开关
10. 增加文件自动刷新开关
11. 增加日志文件路径查询接口
12. 完成编译运行和 GitHub 提交

## 值传递

```cpp
void function(std::string message);
```

值传递会创建参数副本。

函数修改副本时，不会影响调用者原来的对象。

对于 string 等较大对象，复制可能产生额外开销。

## 普通引用

```cpp
void function(std::string &message);
```

引用相当于已有对象的别名。

不会复制对象，函数可以直接修改调用者传入的对象。

## const 引用

```cpp
void function(const std::string &message);
```

const 引用不会复制对象，也不允许函数修改原对象。

适合只读取的大对象参数。

LogManager 的日志消息使用：

```cpp
const std::string &message
```

## this 指针

每个普通成员函数内部都有一个隐含的 this 指针。

```cpp
this->consoleEnabled_
```

表示访问当前对象的 consoleEnabled_ 成员。

例如：

```cpp
logger.setConsoleEnabled(true);
```

进入成员函数时，this 指向 logger 对象。

## *this

this 表示当前对象的地址。

*this 表示当前对象本身。

配置函数返回：

```cpp
LogManager &
```

并执行：

```cpp
return *this;
```

调用者得到的是当前对象的引用，而不是对象副本。

## 链式调用

两个配置函数都返回当前对象引用：

```cpp
logger.setConsoleEnabled(true)
      .setAutoFlush(true);
```

执行过程：

```text
setConsoleEnabled(true)
→ 返回 logger 引用
→ 在同一个 logger 上继续调用 setAutoFlush(true)
```

## const 返回引用

日志路径查询接口：

```cpp
const std::string &getFilePath() const;
```

第一个 const 表示调用者不能通过返回引用修改字符串。

第二个 const 表示该成员函数不会修改对象。

返回引用可以避免复制 filePath_。

返回引用的有效期不能超过 logger 对象的生命周期。

## this 是否必须写

成员函数中可以直接写：

```cpp
consoleEnabled_ = enabled;
```

也可以写：

```cpp
this->consoleEnabled_ = enabled;
```

两种写法都正确。

参数名和成员变量名容易混淆时，this->能够明确表示当前对象成员。

## 实验结果

终端输出可以通过 consoleEnabled_ 控制。

关闭终端输出后，日志仍然会写入文件。

setConsoleEnabled() 和 setAutoFlush() 可以使用链式调用。

## 今日收获

引用可以避免不必要的对象复制。

this 指针指向调用成员函数的当前对象。

返回 *this 可以让多个配置接口连续调用，同时始终操作同一个对象。

## 明日计划

学习 std::string 和 std::vector，使用 vector 保存最近的日志记录，并掌握容器遍历、size、push_back 和范围 for 循环。