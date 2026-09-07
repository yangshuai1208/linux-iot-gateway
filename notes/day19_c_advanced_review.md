# Day19：C语言高级综合复习

## 一、结构体与内存对齐

结构体成员按照声明顺序存放，编译器不能任意改变成员顺序，但会为了满足对齐要求加入填充字节。

计算原则：

1. 每个成员的起始地址必须满足自身对齐要求。
2. 结构体对齐值通常等于最大成员对齐值。
3. 结构体总大小必须是其对齐值的整数倍。

示例：

```c
typedef struct
{
    char type;
    uint32_t sequence;
    uint16_t length;
} MessageA;
```

在题目给定的常见对齐规则下：

- `type`偏移为0。
- `sequence`偏移为4。
- `length`偏移为8。
- `sizeof(MessageA)`为12。

调整成员顺序后可能减少填充，但不能为了节省内存就直接修改通信协议结构。

不能使用`memcmp()`可靠比较普通结构体是否相等，因为填充字节可能不同。跨平台通信时也不能直接发送结构体原始内存，应逐字段编码，并约定字段长度和字节序。

## 二、函数指针与回调函数

函数指针类型：

```c
typedef void (*CommandHandler)(Controller *state,
                               const Command *command);
```

命令分发表：

```c
static const CommandEntry command_table[] =
{
    { CMD_OPEN, handle_open },
    { CMD_STOP, handle_stop }
};
```

区别：

```c
handle_open;                 /* 函数地址 */
handle_open(state, command); /* 调用函数 */
```

回调函数不一定异步执行。它在哪个线程执行、何时执行，由调用回调的代码决定。

## 三、对象生命周期

局部自动变量在函数返回后生命周期结束，不能继续通过其地址访问。

错误示例：

```c
static const Command *saved_command;

static void receive_command(void)
{
    Command command = { .sequence = 19U };
    saved_command = &command;
}
```

函数返回后，`saved_command`成为悬空指针。

正确方法是复制数据：

```c
state->last_command = *command;
```

这里复制的是结构体成员值，而不是保存原指针地址。

注意：如果结构体中包含指针成员，普通结构体赋值只复制指针地址，属于浅拷贝。若要独立保存指针指向的数据，还需要深拷贝。

`const`只能限制通过该指针修改数据，不能延长对象生命周期。

## 四、数组退化与尾后指针

数组作为函数参数时会退化为指针：

```c
void inspect(const CommandEntry table[], size_t count);
```

在`inspect()`内部：

```c
sizeof(table)
```

得到的是指针大小，而不是整个数组大小，所以必须额外传递元素数量。

对于：

```c
int array[5];
```

下面的指针可以合法形成：

```c
int *end = array + 5;
```

它是尾后指针，可以用于比较和表示区间结束位置，但不能解引用。

## 五、未定义行为

常见未定义行为包括：

- 数组越界。
- 解引用尾后指针。
- 使用悬空指针。
- 返回局部变量地址。
- 修改字符串常量。
- 使用不兼容的函数指针调用函数。
- 释放内存后继续访问。
- 有符号整数溢出。
- 对同一对象进行无序的多次读写。

程序某次运行结果正常，不能证明不存在未定义行为。

## 六、数组右旋算法

数组右旋`k`位使用三次翻转：

```c
k %= length;

reverse_range(array, array + length);
reverse_range(array, array + k);
reverse_range(array + k, array + length);
```

例如：

```text
原数组：       1 2 3 4 5
翻转全部：     5 4 3 2 1
翻转前2个：    4 5 3 2 1
翻转剩余部分： 4 5 1 2 3
```

时间复杂度为`O(n)`，额外空间复杂度为`O(1)`。

## 七、MQTT回调与数据所有权

MQTT回调参数中的Payload指针通常只在回调执行期间有效。如果工作线程需要稍后处理，必须复制指针指向的Payload内容，不能只保存地址。

推荐流程：

```text
校验Topic和长度
→ 复制Payload、sequence和时间戳
→ 非阻塞放入有界队列
→ 回调快速返回
→ 工作线程处理
→ 释放内存或归还内存池
```

不能在MQTT网络回调中长时间等待串口或STM32 ACK，否则可能影响后续收包、Keep Alive、PUBACK和断线重连。

## 八、PUBACK与业务ACK

MQTT QoS 1的PUBACK只确认当前MQTT链路的接收方收到了消息，不能证明STM32已经执行动作。

STM32业务ACK应携带：

- `sequence`：对应具体请求。
- `status`：成功、失败或忙碌。
- 必要的错误码。

完整可靠链路还需要超时重传、重复请求去重和幂等处理。

当前项目中应明确真实实现与Linux模拟验证的边界，不能把设计方案描述成已经完成的实机功能。

## 九、Day19练习结果

完成内容：

- 结构体对齐与生命周期综合笔试。
- 命令分发表与函数指针练习。
- 回调数据复制与悬空指针分析。
- 三次翻转实现数组右旋。
- MQTT回调、PUBACK和业务ACK项目口述。

重点错题：

1. 普通结构体成员顺序不会被编译器任意调整。
2. 成员相等不代表`memcmp()`结果一定相等。
3. `int *process(void)`是返回指针的函数，不是函数指针。
4. 尾后指针可以形成，但不能解引用。
5. 回调中保存局部对象地址会产生悬空指针。