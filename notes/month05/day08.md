# 第五个月 Day8：C++ 类与对象及 LogManager Demo

## 今日目标

学习 C++ 类和对象基础，在 Linux IoT Gateway 仓库中实现一个独立的 LogManager 类，并完成多文件编译运行。

## 今日完成内容

1. 学习 class 基本语法
2. 学习对象的创建和成员函数调用
3. 学习 public 和 private 访问权限
4. 学习 enum class
5. 学习作用域解析符 ::
6. 学习头文件与源文件分离
7. 编写 LogManager 类
8. 实现 INFO、WARNING、ERROR 三级日志
9. 使用 g++ 直接编译
10. 使用 Makefile 编译运行
11. 更新项目 README
12. 完成 GitHub 提交

## 项目目录

```text
cpp_modules/log_manager/
├── include/
│   └── log_manager.h
├── src/
│   └── log_manager.cpp
├── demo/
│   └── main.cpp
└── Makefile