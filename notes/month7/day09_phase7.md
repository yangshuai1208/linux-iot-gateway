
# 第七阶段 Day09：可靠通信软件闭环验证

日期：2026-09-27

## 一、今日学习

完成20道嵌入式面试题和16道笔试题。

笔试成绩：15/16。

重点复习C/C++、STM32 DMA、FreeRTOS、SEQ/ACK、
Linux串口、OTA和ROS2线程安全。

## 二、代码训练

1. command_frame_parser.c

实现严格SEQ命令解析，包含前缀、数字溢出、
命令合法性和结束符校验。

2. priority_tx_queue.cpp

练习有界线程安全队列、STOP优先入队、
mutex、condition_variable和安全退出。

两题已完成代码修正，实际编译运行结果以终端记录为准。

## 三、ROS2模拟闭环

测试环境：WSL Ubuntu 22.04、ROS2 Humble。

模拟器：mock_stm32.py。

验证流程：

1. 创建PTY模拟串口。
2. ROS2节点成功打开PTY。
3. 发布GRAB命令。
4. 首次ACK模拟丢失。
5. RetryPolicy使用原SEQ重传。
6. 模拟器返回IN_PROGRESS。
7. 模拟器返回OK。
8. /hand_status查询最终状态。

关键日志：

RX: SEQ:1 CMD:HAND_GRAB
Simulated ACK loss: 1
RX: SEQ:1 CMD:HAND_GRAB
TX: ACK:1 IN_PROGRESS
TX: ACK:1 OK

ROS2最终结果：

SEQ:1 STATUS:SUCCEEDED

结论：Linux/ROS2模拟可靠通信闭环通过。

## 四、尚未验证

STM32真实串口通信、执行端ACK缓存、
STOP抢占、实际舵机动作及ESP32真实OTA。

以上统一安排到Day10实机验证。
