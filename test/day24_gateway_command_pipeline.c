#include <assert.h>
#include <stdio.h>
#include <string.h>

static int is_valid_command(const char *cmd)
{
    /*
     * TODO 1：
     * OPEN / GRAB / RELEASE / STOP
     * 返回1。
     *
     * 其他返回0。
     */
    if(cmd==NULL)
    {
        return 0;
    }
    if(strcmp(cmd,"OPEN")==0||strcmp(cmd,"GRAB")==0||strcmp(cmd,"RELEASE")==0||strcmp(cmd,"STOP")==0)
    {
        return 1;
    }

    return 0;
}

static int validate_mqtt_command(
    const char *device,
    const char *cmd,
    unsigned int seq
)
{
    /*
     * TODO 2：
     *
     * device/cmd == NULL
     * → -1
     *
     * device != "glasses01"
     * → -2
     *
     * cmd非法
     * → -3
     *
     * seq == 0
     * → -4
     *
     * 全部正确
     * → 0
     */
    if(device==NULL||cmd==NULL)
    {
        return -1;
    }
    if(strcmp(device,"glasses01")!=0)
    {
        return -2;
    }
    if(!is_valid_command(cmd))
    {
        return -3;
    }
    if(seq==0)
    {
        return -4;
    }

    return 0;
}

static int command_to_uart(
    const char *cmd,
    char *uart_buf,
    size_t uart_buf_size
)
{
    /*
     * TODO 3：
     * 参数检查。
     */
    if(cmd==NULL||uart_buf==NULL||uart_buf_size==0)
    {
        return -1;
    }
    const char *uart_cmd = NULL;

    /*
     * TODO 4：
     *
     * OPEN
     * → HAND_OPEN\n
     *
     * GRAB
     * → HAND_GRAB\n
     *
     * RELEASE
     * → HAND_RELEASE\n
     *
     * STOP
     * → HAND_STOP\n
     */
    if(strcmp(cmd,"OPEN")==0)
    {
        uart_cmd="HAND_OPEN\n";
    }
    else if(strcmp(cmd,"GRAB")==0)
    {
        uart_cmd="HAND_GRAB\n";
    }
    else    if(strcmp(cmd,"RELEASE")==0)
    {
        uart_cmd="HAND_RELEASE\n";
    }
    else    if(strcmp(cmd,"STOP")==0)
    {
        uart_cmd="HAND_STOP\n";
    }
    else
    {
        return -3;
    }
    /*
     * TODO 5：
     * 判断目标缓冲区是否足够。
     *
     * 不够返回 -5。
     */
    if(strlen(uart_cmd)+1>uart_buf_size)
    {
        return -5;
    }
    /*
     * TODO 6：
     * 拷贝到 uart_buf。
     */
    strcpy(uart_buf,uart_cmd);
    return 0;
}

int process_mqtt_command(
    const char *device,
    const char *cmd,
    unsigned int seq,
    char *uart_buf,
    size_t uart_buf_size
)
{
    /*
     * TODO 7：
     *
     * 第一步：
     * validate_mqtt_command()
     *
     * 如果失败：
     * 直接返回对应错误码。
     *
     * 第二步：
     * command_to_uart()
     *
     * 返回最终结果。
     */
     int ret = validate_mqtt_command(
        device,
        cmd,
        seq
        );

    if (ret != 0)
    {
        return ret;
    }
    
    return command_to_uart(
        cmd,
        uart_buf,
        uart_buf_size
    );

}

int main(void)
{
    char buf[32];

    assert(
        process_mqtt_command(
            "glasses01",
            "GRAB",
            15,
            buf,
            sizeof(buf)
        ) == 0
    );

    assert(
        strcmp(
            buf,
            "HAND_GRAB\n"
        ) == 0
    );

    assert(
        process_mqtt_command(
            "glasses01",
            "STOP",
            20,
            buf,
            sizeof(buf)
        ) == 0
    );

    assert(
        strcmp(
            buf,
            "HAND_STOP\n"
        ) == 0
    );

    assert(
        process_mqtt_command(
            "unknown",
            "GRAB",
            15,
            buf,
            sizeof(buf)
        ) == -2
    );

    assert(
        process_mqtt_command(
            "glasses01",
            "JUMP",
            15,
            buf,
            sizeof(buf)
        ) == -3
    );

    assert(
        process_mqtt_command(
            "glasses01",
            "OPEN",
            0,
            buf,
            sizeof(buf)
        ) == -4
    );

    char small_buf[4];

    assert(
        process_mqtt_command(
            "glasses01",
            "GRAB",
            15,
            small_buf,
            sizeof(small_buf)
        ) == -5
    );

    printf(
        "day24 gateway command pipeline tests passed\n"
    );

    return 0;
}