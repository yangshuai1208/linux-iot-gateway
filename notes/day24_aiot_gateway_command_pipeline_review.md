# Day24：AIoT 网关命令链路与协议校验复习

## 一、今日目标

今天围绕 AIoT 智能眼镜 + Linux Gateway + STM32F407 灵动手项目，重点复习一条控制命令从 ESP32-S3 到机械手执行端的完整链路。

核心链路：

```text
MPU6050
→ ESP32-S3
→ Gesture 识别
→ Command 映射
→ WiFi / MQTT
→ Linux Gateway
→ device / cmd / seq 校验
→ UART 命令转换
→ STM32F407
→ I2C
→ PCA9685
→ PWM
→ MG90S × 5