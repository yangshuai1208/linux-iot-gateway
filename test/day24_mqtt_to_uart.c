#include <assert.h>
#include <stdio.h>
#include <string.h>

int command_to_uart(
    const char *cmd,
    char *uart_buf,
    size_t uart_buf_size
)
{
    /*
     * TODO 1：
     * 检查 cmd、uart_buf 是否为 NULL，
     * uart_buf_size 是否为 0。
     */
    if(cmd==NULL||uart_buf==NULL||uart_buf_size==0)
    {
        return -1;
    }
    const char *uart_cmd=NULL;
    /*
     * TODO 2：
     * OPEN
     * GRAB
     * RELEASE
     * STOP
     *
     * 分别映射为：
     *
     * HAND_OPEN\n
     * HAND_GRAB\n
     * HAND_RELEASE\n
     * HAND_STOP\n
     *
     * 非法命令返回 -2。
     */
    if(strcmp(cmd,"OPEN")==0)
    {
        uart_cmd="HAND_OPEN\n";
    }
    else  if(strcmp(cmd,"GRAB")==0)
    {
        uart_cmd="HAND_GRAB\n";
    }
      
    else   if(strcmp(cmd,"RELEASE")==0)
    {
        uart_cmd="HAND_RELEASE\n";
    }
    else  if(strcmp(cmd,"STOP")==0)
    {
        uart_cmd="HAND_STOP\n";
    }
    else  
    {
        return -2;
    }
    /*
     * TODO 3：
     * 判断 uart_buf 是否足够。
     *
     * 注意：
     * strlen() 不包含字符串结尾 '\0'。
     */
    if(strlen(uart_cmd)+1>uart_buf_size)
    {
        return -3;
    }
    /*
     * TODO 4：
     * 将最终命令复制到 uart_buf。
     */
    strcpy(uart_buf,uart_cmd);

    return 0;
}

int main(void)
{
    char buf[32];

    assert(
        command_to_uart(
            "OPEN",
            buf,
            sizeof(buf)
        ) == 0
    );
    assert(strcmp(buf, "HAND_OPEN\n") == 0);

    assert(
        command_to_uart(
            "GRAB",
            buf,
            sizeof(buf)
        ) == 0
    );
    assert(strcmp(buf, "HAND_GRAB\n") == 0);

    assert(
        command_to_uart(
            "RELEASE",
            buf,
            sizeof(buf)
        ) == 0
    );
    assert(strcmp(buf, "HAND_RELEASE\n") == 0);

    assert(
        command_to_uart(
            "STOP",
            buf,
            sizeof(buf)
        ) == 0
    );
    assert(strcmp(buf, "HAND_STOP\n") == 0);

    assert(
        command_to_uart(
            "JUMP",
            buf,
            sizeof(buf)
        ) == -2
    );

    assert(
        command_to_uart(
            NULL,
            buf,
            sizeof(buf)
        ) == -1
    );

    char small_buf[4];

    assert(
        command_to_uart(
            "GRAB",
            small_buf,
            sizeof(small_buf)
        ) == -3
    );

    printf(
        "day24 mqtt to uart tests passed\n"
    );

    return 0;
}