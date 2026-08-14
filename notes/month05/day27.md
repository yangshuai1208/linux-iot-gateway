# 第五个月 Day27：Lambda 与 STL Algorithm

## Lambda 基本形式

[capture](parameters) -> return_type
{
    body
}

## 捕获列表

### []

不捕获局部变量。

### [value]

按值捕获。

Lambda 内保存变量的一份值。

### [&value]

按引用捕获。

可以修改外部原变量。

### [=]

使用到的局部变量默认按值捕获。

### [&]

使用到的局部变量默认按引用捕获。

### [this]

在成员函数中捕获当前对象。

## Predicate

用于判断条件的可调用对象。

例如：

record.accepted == false

condition_variable 的 wait predicate 也是谓词。

## transform

对范围中的元素进行转换。

项目中用于：

"grab"
→ "GRAB"

## find_if

找到第一个满足条件的元素。

## count_if

统计满足条件的元素数量。

## Lambda 本质

Lambda 可以理解为编译器生成的匿名函数对象，
内部通过 operator() 实现调用。

## 项目应用

ProtocolParser：
transform + lambda

Producer/Consumer：
condition_variable wait predicate

Gateway数据统计：
find_if / count_if

## 工程原则

Lambda 保持短小。

捕获尽量明确。

避免无意义地使用 [&] 捕获全部变量。