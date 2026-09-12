#include <assert.h>
#include <stdio.h>
#include <string.h>

static int is_valid_command(const char *cmd)
{
    /*
     * TODO 1：
     *
     * 如果 cmd 是：
     * OPEN
     * GRAB
     * RELEASE
     * STOP
     *
     * 返回 1。
     *
     * 否则返回 0。
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

int validate_mqtt_command(
    const char *device,
    const char *cmd,
    unsigned int seq
)
{
    /*
     * TODO 2：
     * device 或 cmd 为 NULL
     * 返回 -1。
     */
    if(device==NULL||cmd==NULL)
    {
        return -1;
    }

    /*
     * TODO 3：
     * device 必须等于 "glasses01"。
     *
     * 否则返回 -2。
     */
    if(strcmp(device,"glasses01")!=0)
    {
        return -2;
    }

    /*
     * TODO 4：
     * 调用 is_valid_command()。
     *
     * cmd 非法返回 -3。
     */
    if(!is_valid_command(cmd))
    {
        return -3;
    }


    /*
     * TODO 5：
     * seq == 0
     *
     * 返回 -4。
     */
    if(seq==0)
    {
        return -4;
    }
    return 0;
}

int main(void)
{
    assert(
        validate_mqtt_command(
            "glasses01",
            "GRAB",
            15
        ) == 0
    );

    assert(
        validate_mqtt_command(
            "glasses01",
            "OPEN",
            1
        ) == 0
    );

    assert(
        validate_mqtt_command(
            NULL,
            "GRAB",
            15
        ) == -1
    );

    assert(
        validate_mqtt_command(
            "glasses01",
            NULL,
            15
        ) == -1
    );

    assert(
        validate_mqtt_command(
            "unknown_device",
            "GRAB",
            15
        ) == -2
    );

    assert(
        validate_mqtt_command(
            "glasses01",
            "JUMP",
            15
        ) == -3
    );

    assert(
        validate_mqtt_command(
            "glasses01",
            "STOP",
            0
        ) == -4
    );

    printf(
        "day24 mqtt validation tests passed\n"
    );

    return 0;
}