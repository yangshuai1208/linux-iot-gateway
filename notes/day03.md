# Day03 学习记录

## 今日目标

* 找到之前嵌入式 Linux 阶段完成的网关项目代码
* 整理 `linux-iot-gateway` 正式项目仓库
* 补充 Linux IoT Gateway 项目 README
* 写清楚串口接收、TCP 转发、MQTT 转发、日志模块
* 完成 GitHub 推送准备

## 今日完成

* 在 Ubuntu / WSL 中找到了之前 Linux 阶段项目目录：

```text
/home/yang/linux-learning-submit
```

* 确认网关相关代码主要分布在以下目录：

```text
day21_gateway_v1
day23_gateway_mqtt
day24_gateway_module
day25_gateway_datasource
day26_gateway_applog
day27_gateway_stats
day28_gateway_project_summary
day29_gateway_resume_interview
```

* 明确 `linux-iot-gateway` 不是重新从零开始写，而是把之前 Day21-Day29 的网关代码整理成正式秋招项目。
* 新建或准备整理正式仓库：

```text
/home/yang/linux-iot-gateway
```

* 梳理了 Linux IoT Gateway 的核心功能：

```text
STM32 下位机
    ↓ UART 串口
Linux IoT Gateway
    ↓ TCP / MQTT / 日志
上位机 / 云端 / 后续智能眼镜灵动手系统
```

* 整理了 README 中需要补充的核心内容：

  * 项目简介
  * 项目功能
  * 系统架构
  * 模块划分
  * 编译方式
  * 运行方式
  * 测试方式
  * 面试可讲点

## 今日代码提交

* commit: docs: update linux iot gateway readme

## 实验现象

* Ubuntu 中成功定位到之前 Linux 学习总仓库 `linux-learning-submit`
* 找到了多个与网关项目相关的阶段目录
* 明确了 `linux-iot-gateway` 项目后续整理方向
* 准备将分散的串口、TCP、MQTT、日志代码整理到正式项目仓库
* GitHub 推送时遇到 HTTPS Token 权限问题，需要重新生成具备仓库写权限的 Token

## 遇到的问题

1. 现象：一开始不知道 `linux-iot-gateway` 项目存放在哪里

2. 原因：之前是在嵌入式 Linux 阶段按 Dayxx 目录完成的，不是单独创建的正式仓库

3. 解决：使用 `find` 命令搜索 Linux、gateway、iot 相关目录，最终定位到 `/home/yang/linux-learning-submit`

4. 现象：GitHub push 时出现 403 权限错误

5. 原因：Token 权限不足，或者 Token 没有授权 `linux-iot-gateway` 仓库的写权限

6. 解决：重新生成 GitHub Personal Access Token，Repository access 选择 `linux-iot-gateway`，Repository permissions 中设置 `Contents: Read and write`

## 今日知识点总结

### 1. Linux 项目查找

今天使用了 `find` 命令查找之前的项目目录：

```bash
find ~ -maxdepth 4 -type d \( -iname "*linux*" -o -iname "*gateway*" -o -iname "*iot*" \) 2>/dev/null
```

含义：

* `find`：查找文件或目录
* `~`：当前用户家目录
* `-maxdepth 4`：最多查找 4 层目录
* `-type d`：只查找目录
* `-iname`：忽略大小写匹配名称
* `2>/dev/null`：屏蔽无权限等错误输出

### 2. Linux IoT Gateway 项目定位

之前的网关项目不是一个单独仓库，而是分散在：

```text
/home/yang/linux-learning-submit/day21_gateway_v1
/home/yang/linux-learning-submit/day23_gateway_mqtt
/home/yang/linux-learning-submit/day24_gateway_module
/home/yang/linux-learning-submit/day25_gateway_datasource
/home/yang/linux-learning-submit/day26_gateway_applog
/home/yang/linux-learning-submit/day27_gateway_stats
/home/yang/linux-learning-submit/day28_gateway_project_summary
/home/yang/linux-learning-submit/day29_gateway_resume_interview
```

后续需要将这些内容整理成一个正式项目：

```text
linux-iot-gateway
```

### 3. Linux 网关核心模块

Linux IoT Gateway 项目主要包括四个核心模块：

| 模块             | 功能                        |
| -------------- | ------------------------- |
| serial 串口模块    | 接收 STM32 下位机通过 UART 上传的数据 |
| tcp_server 模块  | 将数据通过 TCP 转发给客户端          |
| mqtt_client 模块 | 将数据发布到 MQTT Broker        |
| log 日志模块       | 记录系统运行状态和异常信息             |

### 4. 串口模块

Linux 下串口本质是设备文件，例如：

```text
/dev/ttyUSB0
/dev/ttyUSB1
/dev/ttyACM0
```

串口通信基本流程：

```text
open 打开串口设备
termios 配置波特率、数据位、停止位、校验位
read 读取数据
write 发送数据
close 关闭设备
```

### 5. TCP Server 模块

TCP 服务端基本流程：

```text
socket
bind
listen
accept
send / recv
close
```

在网关项目中，TCP Server 用于把串口接收到的数据转发给上位机或客户端。

### 6. MQTT 模块

MQTT 是物联网常用的轻量级发布订阅协议。

核心概念：

```text
Broker：消息服务器
Topic：主题
Publish：发布消息
Subscribe：订阅消息
```

示例：

```text
topic: gateway/env
payload: {"temp":25.3,"humi":48.0}
```

### 7. 日志模块

日志模块的作用：

* 记录串口是否收到数据
* 记录 TCP 是否转发成功
* 记录 MQTT 是否发布成功
* 记录异常命令和错误状态
* 方便调试和面试展示

示例日志：

```text
[2026-06-16 20:30:12] serial recv TEMP=25.3,HUMI=48.0
[2026-06-16 20:30:13] mqtt publish topic=gateway/env status=OK
```

### 8. GitHub Token 权限

GitHub 通过 HTTPS 推送代码时，密码位置不能输入 GitHub 登录密码，需要输入 Personal Access Token。

Fine-grained Token 推荐配置：

```text
Repository access:
Only select repositories
  - linux-iot-gateway

Repository permissions:
Contents: Read and write
Metadata: Read-only
```

注意：Token 不能直接粘到终端执行。只有当终端提示：

```text
Password for 'https://github.com':
```

这时候才粘贴 Token。

## 面试可讲点

* 我做了一个基于 Linux C 的物联网边缘网关项目。
* 项目负责接收 STM32 下位机通过串口上传的数据。
* 网关可以通过 TCP 或 MQTT 将数据转发给上位机或云端。
* 项目中设计了日志模块，用于记录串口接收、网络转发和异常状态。
* 串口模块使用 `open`、`termios`、`read`、`write` 完成通信。
* TCP Server 使用 `socket`、`bind`、`listen`、`accept`、`send` 完成数据转发。
* MQTT 用于物联网场景下的轻量级消息发布与订阅。
* 该网关后续可以作为 AIoT 智能眼镜控制具身智能灵动手系统的中间协议转换层。

## 今日总结

今天完成了 `linux-iot-gateway` 项目的定位和整理规划。之前的网关代码分散在 `linux-learning-submit` 的多个 Dayxx 目录中，今天通过 `find` 命令找到了这些项目内容，并明确了后续整理方向。

今天的重点不是重新写代码，而是把已有 Linux 阶段成果整理成正式项目仓库，使其具备 GitHub 展示价值、README 可读性和面试讲解价值。

## 明日计划

* 补充 Linux 网关运行说明和测试记录
* 写清楚编译命令和运行命令
* 整理串口测试现象
* 整理 TCP 测试现象
* 整理 MQTT 测试现象
* 补充 `notes/day04.md`
