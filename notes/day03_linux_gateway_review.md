# 第六阶段 Day 3：Linux Gateway与Linux串口复习

## 一、今日目标

1. 复盘Linux Gateway在三端系统中的作用。
2. 审计`linux-iot-gateway`真实代码。
3. 修复JSON格式和缓冲区截断问题。
4. 补全Linux真实串口数据源。
5. 复习`open/read/close/termios`。
6. 复习MQTT QoS和消息可靠性。
7. 完成环形链表和寻找环入口算法。
8. 完成项目追问、选择判断和代码分析。

---

## 二、Linux Gateway架构

系统由三个部分组成：

```text
ESP32-S3采集端
→ WiFi/MQTT
→ Linux Gateway
→ 串口协议
→ STM32F407执行端