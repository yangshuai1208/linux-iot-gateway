```markdown
# 测试记录

## 一、测试目标

本测试记录用于说明 Linux IoT Gateway 的基本运行现象，包括：

- 串口接收测试
- TCP 转发测试
- MQTT 发布测试
- 日志记录测试

## 二、串口测试

### 测试命令

查看串口设备：

```bash
ls /dev/ttyUSB*