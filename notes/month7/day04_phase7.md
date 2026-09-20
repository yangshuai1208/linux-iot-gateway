# 第七阶段 Day04 学习总结

日期：2026-09-20

## 一、今日结论

Day04 继续围绕“面试笔试强化 + 旗舰项目工程化升级”展开。

今日完成内容：

1. 完成 12 道综合面试题训练
2. 完成 20 道综合笔试题训练
3. 完成 C 位操作代码题 `bit_register.c`
4. 完成 C++ 原子计数器代码题 `atomic_counter.cpp`
5. STM32F407 执行端完成“非阻塞动作状态机”软件重构，Keil 编译 0 Error
6. ESP32-S3 OTA 增加 rollback / 新固件确认框架，`idf.py build` 成功
7. ROS2 完成控制命令到 STM32 UART 文本协议的映射，并实际验证：
   - GRAB → HAND_GRAB
   - OPEN → HAND_OPEN
8. ESP32-S3 OTA 板载测试由于未携带开发板，顺延 Day05

今天最大的工程升级是：

原来的机械手动作使用：

`while + HAL_Delay()`

改造成：

`start_action() + update() + HAL_GetTick()`

为后续 STOP 抢占、FSR 压力反馈、UART DMA/IDLE、Watchdog 和 Fault 机制打下基础。

---

# 二、面试知识点复习

## 1. static

文件作用域：

```c
static int g_count;
static void helper(void);
作用：

具有内部链接
只能在当前翻译单元 / .c 文件中访问

局部静态变量：

void func(void)
{
    static int count = 0;
}

特点：

作用域仍然只在函数内部
生命周期贯穿程序整个运行期间
只初始化一次

存储位置：

零初始化或未显式初始化：通常位于 .bss
非零初始化：通常位于 .data

面试回答：

文件作用域的 static 变量和函数具有内部链接，只能在当前翻译单元访问；局部 static 变量的作用域仍然局限于函数，但存储周期贯穿整个程序，只初始化一次。

2. const 指针
const int *p;
int const *p;

表示：

指向常量的指针，不能通过 p 修改目标数据，但 p 可以改变指向。

int *const p;

表示：

常量指针，p 不能改变指向，但可以修改所指数据。

const int *const p;

表示：

指针和所指数据都不能通过 p 修改。

记忆：

const 在 * 左边：数据不能改
const 在 * 右边：指针不能改

3. C++ virtual 与运行时多态

示例：

Base *p = new Derived;
p->run();

如果 run() 不是 virtual：

根据指针的静态类型 Base* 进行静态绑定。

如果 run() 是 virtual：

根据对象实际类型 Derived 进行运行时动态绑定。

常见编译器通过：

对象 → vptr → vtable → 虚函数地址

实现动态分派。

注意：

vptr / vtable 是常见实现机制，不是 C++ 标准强制规定的具体实现。

如果存在：

Base *p = new Derived;
delete p;

基类析构函数应该声明为：

virtual ~Base() = default;

否则通过基类指针删除派生类对象会产生未定义行为。

4. UART DMA + IDLE

DMA 的作用：

自动完成 UART 外设与内存之间的数据搬运，减少 CPU 逐字节参与。

IDLE 的作用：

辅助判断一段可变长 UART 数据什么时候暂时接收结束。

典型流程：

UART
 ↓
DMA Buffer
 ↓
IDLE事件
 ↓
计算本次新增数据长度
 ↓
Ring Buffer
 ↓
协议解析

例如 DMA Buffer 大小为 128 Byte，但当前一帧只有 37 Byte：

如果只等 DMA 缓冲区填满，会产生不必要等待。

加入 IDLE 后：

收到37字节
↓
总线出现空闲
↓
IDLE触发
↓
立即处理37字节
5. NVIC 与 FreeRTOS 中断优先级

Cortex-M：

优先级数字越小，实际中断优先级越高。

例如：

IRQ_A = 2
IRQ_B = 5

IRQ_A 优先级更高。

FreeRTOS 下不是所有中断都能调用：

xQueueSendFromISR();

原因：

FreeRTOS 需要通过中断优先级屏蔽机制保护内核临界区。优先级过高、能够突破内核屏蔽的中断不能调用 RTOS 内核 API，否则可能破坏内核数据结构的一致性。

configMAX_SYSCALL_INTERRUPT_PRIORITY：

定义允许调用 FreeRTOS FromISR API 的最高中断紧迫程度阈值。

6. FreeRTOS 优先级反转

场景：

L：低优先级任务持有 Mutex

H：高优先级任务需要同一个 Mutex
→ H阻塞

M：中优先级任务持续运行
→ M不断抢占L

L不能及时运行
→ 无法释放Mutex

H一直等待

这就是优先级反转。

Mutex 的优先级继承：

H等待L的Mutex
↓
临时提高L优先级
↓
L尽快运行
↓
释放Mutex
↓
恢复原优先级
↓
H继续执行

注意：

FreeRTOS Mutex 支持优先级继承，普通 Binary Semaphore 不具备同样的优先级继承语义。

7. select vs epoll

连接数量较少，例如 3～5 个 fd：

可以使用 select，实现简单，性能足够。

连接数量达到几千：

更适合使用 epoll。

select：

每次调用需要处理 fd 集合
返回后还需要遍历检查
通常受 FD_SETSIZE 限制

epoll：

通过 epoll_ctl() 注册感兴趣的 fd
epoll_wait() 返回已经就绪的事件
大量连接、少量活跃场景下更合适

ET 模式通常使用非阻塞 fd，并一直 read 到：

EAGAIN

否则可能遗漏后续数据通知。

8. TCP ACK 与业务 ACK

TCP ACK 只能说明：

字节已经可靠到达对端 TCP 协议栈。

它不能证明：

Linux解析成功
STM32收到UART
STM32识别GRAB
机械手完成动作

所以机械手链路仍然需要应用层：

seq
+
ACK
+
timeout
+
retry
+
dedup

一句话：

TCP ACK 表示“字节到了”，业务 ACK 表示“这条命令是否被正确识别和执行”。

三、代码题
1. bit_register.c

实现：

void set_bit(uint32_t *reg, uint8_t bit);
void clear_bit(uint32_t *reg, uint8_t bit);
void toggle_bit(uint32_t *reg, uint8_t bit);
bool get_bit(uint32_t reg, uint8_t bit);

核心位操作：

*reg |= (1U << bit);

置位。

*reg &= ~(1U << bit);

清零。

*reg ^= (1U << bit);

翻转。

return (reg & (1U << bit)) != 0U;

读取。

必须检查：

bit >= 32U

避免发生：

1U << 32

这种非法移位。

面试回答：

寄存器单个位控制一般通过位掩码实现。置位用 OR，清零用 AND 加取反，翻转用 XOR，读取用 AND 判断，同时要限制 bit 范围避免移位超出类型宽度。

2. atomic_counter.cpp

使用：

std::atomic<int>
std::thread
fetch_add()
join()
load()

4 个线程：

每个线程 +100000

最终：

counter = 400000

fetch_add()：

原子完成读、修改、写操作，避免多个线程对普通整数自增时产生数据竞争。

join()：

主线程等待子线程执行完成。

注意：

std::atomic 更适合：

计数器
状态标志
简单共享变量

但下面这种复合逻辑：

if (!queue.empty())
{
    data = queue.front();
    queue.pop();
}

通常仍然需要 Mutex，因为必须保证多个步骤组成的整个临界区具有一致性。

四、STM32F407 执行端升级①：非阻塞动作状态机
1. 原来的问题

原 hand_servo.c 中舵机平滑移动使用：

while (current_angles[index] != target_angle)
{
    ...
    HAL_Delay(SERVO_MOVE_DELAY_MS);
}

并且 5 个手指依次调用这一阻塞函数。

问题：

开始动作
↓
while
↓
HAL_Delay
↓
完整动作结束
↓
main才重新得到执行机会

导致动作期间不方便及时处理：

新 UART 命令
STOP
FSR 压力反馈
Watchdog
Fault
2. 新增动作状态

hand_servo.h 新增：

typedef enum
{
    HAND_MOTION_IDLE = 0,
    HAND_MOTION_OPENING,
    HAND_MOTION_GRABBING,
    HAND_MOTION_RELEASING,
    HAND_MOTION_STOPPING,
    HAND_MOTION_FAULT
} hand_motion_state_t;

并新增：

HAL_StatusTypeDef hand_servo_start_action(hand_action_t action);

HAL_StatusTypeDef hand_servo_update(void);

uint8_t hand_servo_is_busy(void);

hand_motion_state_t hand_servo_get_state(void);

当前头文件已经具备相应状态和非阻塞接口。

3. start_action()

职责：

设置 target_angles
设置 motion_state
记录 last_move_tick
立即返回

例如：

HAND_ACTION_GRAB
↓
target = 120°
↓
state = GRABBING
↓
return

不再在函数内部完成整个动作。

4. update()

主循环不断调用：

hand_servo_update();

内部使用：

HAL_GetTick();

判断是否达到：

SERVO_MOVE_DELAY_MS = 20ms

每次只把当前角度向目标移动：

5°

然后立即返回。

当前实现已经按照这种方式遍历 5 个舵机，并在 PCA9685 写失败时进入 HAND_MOTION_FAULT。

5. 新结构优势

原来：

拇指完整移动
↓
食指完整移动
↓
中指完整移动
...

现在：

拇指 +5°
食指 +5°
中指 +5°
无名指 +5°
小指 +5°
↓
返回主循环

下一次 Tick 再继续。

因此：

动作过程不再长期占用主流程
五个舵机更接近同步渐进运动
主循环能够持续运行
为 STOP 抢占打基础
为 FSR 闭环打基础
为 Watchdog / Fault 打基础
为 UART DMA/IDLE 升级打基础

主循环已经改为使用 hand_servo_start_action() 启动作业，并通过 hand_servo_update() 推进动作，而不是直接等待阻塞式动作完成。

6. 今日验证结果
Keil Build：
0 Error

所以当前能够准确描述为：

STM32F407 执行端已完成非阻塞动作状态机的软件重构，并通过 Keil 编译验证。

目前不能描述为：

已完成机械手实机非阻塞动作验证。

因为今天没有进行真实舵机板载测试。

7. 面试回答

原来的机械手动作函数内部使用 while 和 HAL_Delay 平滑调整舵机角度，在动作执行期间主控制流程长时间停留在函数内部，不利于及时处理 STOP、新 UART 命令、压力反馈和健康检查。后来我将动作拆成 start_action 和 update 两个阶段，start 只记录目标角度和状态，主循环通过 HAL_GetTick 周期性调用 update，每次只推进一步然后立即返回。这样动作执行过程中系统仍然能够持续处理通信和安全事件，也为后续 STOP 抢占、FSR 反馈和 Fault 机制打下基础。

五、OTA Rollback 软件升级
1. 原有 OTA 流程

已经实现：

esp_ota_get_next_update_partition()
↓
esp_ota_begin()
↓
esp_ota_write()
↓
esp_ota_end()
↓
esp_ota_set_boot_partition()

并具有：

factory
ota_0
ota_1

分区结构。

2. 双 OTA 分区不等于自动回滚

如果：

V1稳定运行
↓
升级到V2
↓
V2能够启动
↓
但5秒后崩溃

仅仅保留两个 OTA 分区还不够。

真正需要：

V2首次启动
↓
PENDING_VERIFY
↓
运行健康检查

成功
→ mark valid

失败
→ mark invalid
→ rollback
3. 今日增加的新固件确认框架

新增：

ota_manager_check_and_confirm_app();

核心 API：

esp_ota_get_state_partition();

检查当前 OTA image 状态。

如果：

ESP_OTA_IMG_PENDING_VERIFY

则执行软件自检。

成功：

esp_ota_mark_app_valid_cancel_rollback();

失败：

esp_ota_mark_app_invalid_rollback_and_reboot();
4. 当前 self-test 状态

目前：

bool self_test_ok = true;

只是 Day04 的软件框架占位。

还没有实现真实：

NVS检查
WiFi检查
MQTT检查
MPU6050检查
关键任务检查

因此不能描述为：

已完成完整 OTA 健康检查。

准确描述：

已搭建 OTA rollback 和新固件确认的软件框架。

5. 今日 OTA 验证

ESP-IDF 环境一度出现：

idf.py: command not found

以及：

/tmp/esp_idf_activate_yang/
unexpected EOF

处理：

rm -rf /tmp/esp_idf_activate_yang
source ~/esp/esp-idf/export.sh

恢复 ESP-IDF 环境。

随后：

idf.py build

结果：

Project build complete.

因此今日可确认：

OTA rollback 软件框架已经编译通过。

不能确认：

真实V1 → V2升级
真实PENDING_VERIFY
真实mark valid
真实rollback

这些需要 Day05 板载测试。

6. Day05 OTA 待修改点

今天暂时保留，明天处理：

factory分区下
esp_ota_get_state_partition()
返回 ESP_ERR_NOT_SUPPORTED
的正常处理

以及：

把 bool self_test_ok = true
替换为真实健康检查
7. OTA 面试回答

我的 OTA 使用 factory 加双 OTA 分区。下载阶段通过 esp_ota_begin、write、end 写入新镜像，再通过 set_boot_partition 切换下次启动分区。但双分区本身不等于具备回滚能力，所以我进一步加入 Bootloader rollback 思路。新固件首次启动后先处于待验证状态，完成关键模块自检以后再标记当前 APP 有效；如果自检失败，则把当前镜像标记无效并触发回滚，避免一个能够启动但运行不稳定的新版本被长期保留。

六、ROS2 hand_bridge
1. 当前节点职责

当前 ROS2 节点：

hand_bridge_node

负责：

ROS2上层控制命令
↓
Linux Gateway
↓
STM32控制协议

当前已经设计：

Topic：
/hand_command

Service：
/hand_status

Parameter：
uart_port
baud_rate
2. 控制命令

ROS2 上层：

OPEN
GRAB
RELEASE
STOP

STM32 当前协议：

HAND_OPEN
HAND_GRAB
HAND_RELEASE
HAND_STOP

所以新增：

map_to_uart_command()

完成协议转换。

映射关系：

OPEN
→ HAND_OPEN\r\n

GRAB
→ HAND_GRAB\r\n

RELEASE
→ HAND_RELEASE\r\n

STOP
→ HAND_STOP\r\n
3. 为什么带 \r\n

STM32 当前串口协议逻辑：

\r
→ 忽略

\n
→ 判断一条完整文本命令结束

因此：

HAND_GRAB\r\n

可以正确适配当前 STM32 文本命令解析方式。

4. 今日实际验证

实际发送：

ros2 topic pub --once \
/hand_command \
std_msgs/msg/String \
"{data: 'GRAB'}"

节点输出：

Received command: GRAB
Mapped UART command: HAND_GRAB

发送：

OPEN

节点输出：

Received command: OPEN
Mapped UART command: HAND_OPEN

因此今天可以确认：

ROS2 控制命令到 STM32 UART 文本协议的映射已经实际运行验证成功。

5. 当前边界

目前只是：

ROS2 Topic
↓
Linux hand_bridge
↓
字符串映射

还没有真正执行：

open()
termios
write()

向 STM32 串口发送数据。

因此不能描述：

ROS2 已经实际控制 STM32 机械手。

准确描述：

ROS2 到 STM32 控制协议的 Linux Gateway 映射层已经跑通，真实 UART 发送仍待后续接入。

另外：

/hand_status 当前保存的是：

ROS2 节点最近一次收到的合法控制命令。

它还不是：

STM32 返回的真实执行状态。

后续需要：

STM32 ACK / status
↓
UART
↓
Linux Gateway
↓
ROS2 /hand_status

才能形成真实状态闭环。

七、今日主要问题与解决方法
1. ESP-IDF 环境异常

问题：

idf.py command not found

原因：

ESP-IDF activation 临时缓存异常

处理：

rm -rf /tmp/esp_idf_activate_yang
source ~/esp/esp-idf/export.sh

最终：

idf.py build成功
2. GitHub push 网络异常

出现：

Failed to connect to github.com
Recv failure: Connection was reset
Could not resolve host: github.com

本地 Git commit 正常。

网络恢复后重新：

git push

最终推送成功。