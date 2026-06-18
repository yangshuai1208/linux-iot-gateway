# linux-iot-gateway

## 项目简介

本项目是一个基于 Linux C 的物联网网关系统，主要用于接收下位机 STM32 通过串口发送的传感器数据，并将数据转发到 TCP 客户端或 MQTT Broker，同时通过日志模块记录系统运行状态。

项目目标不是简单串口打印，而是模拟真实物联网系统中的边缘网关：负责设备接入、协议转换、数据转发、异常记录和后续系统扩展。

## 项目功能

- 串口接收 STM32 下位机数据
- TCP Server 向客户端转发数据
- MQTT Client 上传传感器数据
- 日志模块记录系统运行状态
- Makefile 一键编译
- 支持模块化代码结构
- 为后续 AIoT 智能眼镜 + 灵动手控制系统提供网关基础

## 系统架构

```mermaid
flowchart TD
    A[STM32 下位机] -->|UART 串口| B[Linux IoT Gateway]

    B --> C[serial 模块]
    B --> D[tcp_server 模块]
    B --> E[mqtt_client 模块]
    B --> F[log 模块]

    C --> G[解析串口数据]
    D --> H[TCP 客户端]
    E --> I[MQTT Broker]
    F --> J[本地日志文件]

    I --> K[云端 / 上位机 / 其他设备]
    
---


```markdown
## 编译与运行

详细说明见：

- [编译与运行说明](docs/build_run.md)
- [测试记录](docs/test_record.md)

基础编译：

```bash
make