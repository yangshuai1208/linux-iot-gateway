
# 第七阶段 Day08：STM32可靠控制、ESP32 OTA与ROS2 ACK闭环

整理日期：2026-09-26

## 一、今日目标

1. 完成嵌入式面试与笔试强化。
2. 完成C语言DMA环形缓冲区题和C++ ACK解析题。
3. 升级STM32执行端的状态机与SEQ缓存。
4. 推进ESP32-S3 V1.0.0到V1.0.1的OTA测试。
5. 将Linux Gateway的SEQ、ACK解析与超时重试接入ROS2。

## 二、面试与笔试复习

### 1. C/C++基础

- 局部变量位于栈上，函数返回后不能返回其地址供外部继续访问。
- RAII通过对象生命周期管理资源，析构函数负责释放文件描述符、锁等资源。
- 互斥锁保护共享数据，原子变量适合独立计数器和退出标志。
- condition_variable等待时需要配合谓词，防止虚假唤醒。
- 线程退出时先通知停止，再join，最后释放线程仍可能访问的资源。

### 2. STM32与DMA

- DMA环形缓冲区需要处理普通区间和跨末尾回绕区间。
- 读取位置与写入位置之间应使用明确的边界条件。
- DMA负责数据搬运，不等于已经完成应用层协议解析。
- 串口接收需要考虑半包、粘包、缓冲区溢出及异常恢复。

### 3. Linux与通信

- write()可能发生短写，需要循环发送剩余字节。
- EINTR表示系统调用被信号中断，可根据场景重试。
- 非阻塞IO出现EAGAIN时，应等待可写条件而不是无限忙等。
- VMIN=0、VTIME=5时，read()返回0可能表示接收超时。
- ACK需要携带SEQ，避免把其他请求的响应误认为当前请求的结果。

### 4. OTA与可靠性

- OTA流程包括获取固件、写入备用分区、校验、切换启动分区和重启。
- 新固件启动成功与OTA下载成功是两回事。
- 回滚与有效性确认需要结合实际配置和启动结果验证。
- 超时重发必须复用原SEQ，执行端通过SEQ缓存降低重复执行风险。
- 重试耗尽表示最终结果未知，不代表设备一定没有执行命令。

## 三、代码题

### 题目一：DMA环形缓冲区区间拆分

文件：seventh_stage/day08/dma_spans.c

实现内容：

- 普通数据区间。
- 跨环形缓冲区末尾的双区间。
- 下标边界检查。
- 使用assert验证结果。

结果：已修正并运行成功。

### 题目二：ACK文本解析器

练习文件：seventh_stage/day08/ack_line_parser.cpp

协议格式：

ACK:<SEQ> <STATUS>

支持状态：

- IN_PROGRESS
- OK
- PREEMPTED
- BUSY
- ERROR

关键处理：

- 去除CRLF。
- 检查ACK前缀。
- 检查SEQ数字格式。
- 防止uint32_t整数溢出。
- 校验失败时不修改输出参数。

工程化实现：

ros2_ws/src/hand_bridge/include/hand_bridge/ack_parser.hpp

ros2_ws/src/hand_bridge/src/ack_parser.cpp

## 四、STM32执行端升级

仓库：stm32_f407_hand_controller

本轮改进：

1. 优化hand_servo_update()，仅在角度变化时写入PCA9685。
2. 修正动作完成时机，避免完成状态额外延迟一个周期。
3. 加强SEQ数字解析与溢出检查。
4. ACK缓存同时记录SEQ与命令。
5. 相同SEQ对应不同命令时返回SEQ_CONFLICT。
6. 保留STOP抢占和非阻塞舵机状态更新。

验证情况：

Keil软件编译通过。

尚未完成真实STM32板载验证。

当前STOP是软件动作控制，不是切断执行器电源的硬件急停。

## 五、ESP32-S3 OTA

仓库：aiot-smart-glasses

固件版本：

V1.0.0 → V1.0.1

已完成：

1. 构建V1.0.0 OTA测试固件。
2. ESP32-S3烧录并成功启动。
3. Wi-Fi成功获取IP地址。
4. 识别factory和ota_0分区。
5. 调整V1.0.0测试流程，在外设初始化前进入OTA入口。

实际日志包含：

OTA test mode: skip peripheral init
Start OTA: V1.0.0 -> V1.0.1

未完成：

- GitHub HTTPS固件下载成功验证。
- OTA写入和启动分区切换验证。
- V1.0.1启动与有效性确认。
- 失败回滚实测。

MQTT连接超时与OTA下载结果需要分别判断，不能混为同一个故障。

以上验证顺延Day09。

## 六、Linux Gateway与ROS2升级

仓库：linux-iot-gateway

ROS2工作空间：ros2_ws

软件包：hand_bridge

### 1. SEQ命令协议

ROS2命令：

OPEN、GRAB、RELEASE、STOP

转换为STM32串口协议：

SEQ:101 CMD:HAND_GRAB\r\n

每条新请求分配SEQ。

超时重传复用原SEQ，不生成新编号。

### 2. ACK解析与UART RX Worker

新增ACK解析模块。

RX Worker负责：

- 接收UART数据。
- 拼接完整文本行。
- 处理分包和多条连续ACK。
- 丢弃超长数据行。
- 根据SEQ更新请求状态。

### 3. RetryPolicy

新增retry_policy.hpp。

请求状态包括：

QUEUED
WAITING_ACK
EXECUTING
RETRY_QUEUED
SUCCEEDED
PREEMPTED
BUSY
ERROR
SUPERSEDED
RESULT_UNKNOWN

测试参数：

初始ACK等待：1500ms
执行完成等待：15000ms
最大重试次数：2次

实际发送成功后开始计时。

收到IN_PROGRESS后进入执行等待。

收到OK后结束请求。

超过重试次数后标记RESULT_UNKNOWN。

### 4. ROS2多线程架构

TX Worker：统一执行UART写入。

RX Worker：接收并解析STM32 ACK。

Retry Worker：检查超时并重新提交原SEQ请求。

ROS2回调：注册请求并入队，不直接阻塞等待ACK。

共享RetryPolicy通过互斥锁保护。

### 5. STOP处理

STOP进入高优先级软件队列。

旧普通命令停止后续重试。

已开始写入串口的数据不能仅通过清空队列撤回。

STOP软件队列优先级不等于硬件安全急停。

### 6. /hand_status服务

按最近一次提交的SEQ查询请求状态。

只有SUCCEEDED对应成功状态。

WAITING_ACK和EXECUTING不代表执行完成。

## 七、实际测试记录

RetryPolicy独立测试：已运行成功。

ROS2 hand_bridge编译：成功。

编译结果：

Finished <<< hand_bridge
Summary: 1 package finished

模拟STM32：

Python伪终端程序已启动。

模拟器显示端口：

/dev/pts/6

ROS2节点尝试打开该端口时出现：

Open UART failed: No such file or directory

因此本轮尚未完成模拟ACK闭环。

不能将ROS2命令发布成功等同于STM32收到并执行命令。

## 八、Day09待办

1. 检查模拟器与ROS2终端是否处于同一个WSL环境，解决伪终端不可见问题。
2. 验证模拟ACK丢失、同SEQ重发、IN_PROGRESS和OK状态流转。
3. STM32实机验证SEQ、ACK、STOP抢占与舵机动作。
4. 继续ESP32-S3真实OTA下载、写入、重启与版本确认。
5. 根据实测结果调整通信超时参数。
6. 补充真实测试日志，完善README。

## 九、今日总结

完成了STM32执行端的软件升级及Linux/ROS2可靠通信模块集成。

RetryPolicy独立测试通过，ROS2工程构建成功。

ESP32 OTA已经进入升级入口，但真实升级未完成。

ROS2模拟器已启动，但由于伪终端打开失败，完整ACK闭环尚未通过。

Day09以真实验证和问题收敛为主，不再重复大规模编写已有模块。
