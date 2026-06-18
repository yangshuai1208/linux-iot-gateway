# Day04 学习记录

## 今日目标

- 补充 Linux IoT Gateway 编译说明
- 补充 Linux IoT Gateway 运行说明
- 整理串口测试现象
- 整理 TCP 测试现象
- 整理 MQTT 测试现象
- 完成 GitHub 提交

## 今日完成

- 新增 docs/build_run.md，说明项目编译、运行、清理和依赖安装方式
- 新增 docs/test_record.md，记录串口、TCP、MQTT 和日志测试现象
- 更新 README.md，补充编译运行说明和测试现象
- 梳理了 Linux 网关在智能眼镜和灵动手系统中的中间层作用
- 完成 GitHub 提交

## 今日代码提交

- commit: docs: add gateway build and test instructions

## 实验现象

- README.md 中可以看到编译和运行方式
- docs/build_run.md 中可以看到 make、make clean、./iot_gateway 等命令说明
- docs/test_record.md 中可以看到串口、TCP、MQTT 的测试输入和预期输出
- 项目文档更加适合 GitHub 展示和面试讲解

## 遇到的问题

1. 现象：Linux 网关测试内容较多，初期 README 不容易写清楚  
2. 原因：项目同时涉及串口、TCP、MQTT、日志等模块  
3. 解决：将运行说明和测试记录拆分到 docs/build_run.md 和 docs/test_record.md，README 只保留核心内容和链接

## 面试可讲点

- Linux 网关通过串口接收 STM32 上传的数据
- TCP Server 可以将数据转发给客户端
- MQTT Client 可以将数据发布到指定 topic
- 日志模块可以记录串口接收、网络转发和异常状态
- Makefile 用于管理 Linux C 多文件编译
- 网关后续可以作为智能眼镜和灵动手之间的协议转换层

## 明日计划

- 设计 AIoT 智能眼镜 + 具身智能灵动手控制系统总架构
- 新建 aiot-embodied-control-system 仓库
- 编写系统级 README 初版
- 确定智能眼镜、Linux 网关、灵动手、ESP32 OTA 模块关系