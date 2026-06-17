
---

### 2. docs/modules.md 内容

复制到 `docs/modules.md`：

```markdown
# 模块说明

## 一、serial 串口模块

串口模块负责和 STM32 下位机通信。

主要功能：

- 打开串口设备
- 配置波特率
- 配置数据位、停止位、校验位
- 读取 STM32 上传的数据
- 将原始数据交给上层处理

常见串口设备：

```text
/dev/ttyUSB0
/dev/ttyUSB1
/dev/ttyACM0