# Day21 C++ 二进制串口协议

## 帧结构

AA 55
LEN
SEQ_H
SEQ_L
CMD
CHECKSUM
0D 0A

## 命令码

0x01 OPEN
0x02 GRAB
0x03 RELEASE
0x04 STOP

## sequence

使用两个字节，大端序：

SEQ_H → 高8位
SEQ_L → 低8位

## checksum

当前采用 XOR：

LEN ^ SEQ_H ^ SEQ_L ^ CMD

用于学习协议校验流程。

实际工程可考虑 CRC-8 / CRC-16。

## encode

结构化 Frame
→ 字节数组

## decode

字节数组
→ 检查帧头
→ 检查长度
→ 检查帧尾
→ 检查 checksum
→ 提取 sequence
→ 提取 command

## 粘包拆包

串口是字节流。

一次 read 不等于一帧。

所以需要：

buffer
→ 找帧头
→ 看长度
→ 等完整帧
→ 校验
→ 提取

## 当前边界

这是二进制协议 V2 实验 Demo。

当前正式项目链路尚未整体切换为该协议。