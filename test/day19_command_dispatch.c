/* Day19：命令分发表与回调。仅验证本地逻辑，不操作串口或硬件。 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum
{
    CMD_OPEN = 1,
    CMD_STOP = 2
} CommandType;

typedef struct
{
    unsigned int sequence;
    CommandType type;
} Command;

typedef struct
{
    bool stopped;
    bool has_command;
    Command last_command; /* 保存数据副本，不保存调用者的指针。 */
} Controller;

typedef void (*CommandHandler)(Controller *state, const Command *command);

typedef struct
{
    CommandType type;
    CommandHandler handler;
} CommandEntry;

static void handle_open(Controller *state, const Command *command)
{
    state->last_command = *command;
    state->has_command = true;
    state->stopped = false;
}

static void handle_stop(Controller *state, const Command *command)
{
    state->last_command = *command;
    state->has_command = true;
    state->stopped = true;
}

static const CommandEntry command_table[] =
{
    { CMD_OPEN, handle_open },
    { CMD_STOP, handle_stop }
};

/* 返回值：0=成功，-1=空指针参数，-2=未知命令。 */
static int dispatch_command(Controller *state, const Command *command)
{
    if ((state == NULL) || (command == NULL))
    {
        return -1;
    }

    const size_t count = sizeof(command_table) / sizeof(command_table[0]);

    for (size_t i = 0; i < count; ++i)
    {
        if (command_table[i].type == command->type)
        {
            command_table[i].handler(state, command);
            return 0;
        }
    }

    return -2;
}

static int receive_once(Controller *state)
{
    Command command = { .sequence = 19U, .type = CMD_OPEN };
    return dispatch_command(state, &command);
}

int main(void)
{
    Controller state = { .stopped = true };

    int result = receive_once(&state);
    /* receive_once 已返回，它的局部对象已结束生命周期。 */
    assert(result == 0);
    assert(state.has_command);
    assert(!state.stopped);
    assert(state.last_command.sequence == 19U);
    assert(state.last_command.type == CMD_OPEN);
    puts("[PASS] OPEN callback and saved value after return");

    Command stop = { .sequence = 20U, .type = CMD_STOP };
    result = dispatch_command(&state, &stop);
    assert(result == 0);
    assert(state.stopped);
    assert(state.last_command.sequence == 20U);
    assert(state.last_command.type == CMD_STOP);
    puts("[PASS] STOP callback");

    stop.sequence = 999U;
    assert(state.last_command.sequence == 20U);
    puts("[PASS] changing source does not change saved value");

    Command unknown = { .sequence = 21U, .type = (CommandType)99 };
    result = dispatch_command(&state, &unknown);
    assert(result == -2);
    assert(state.stopped);
    assert(state.has_command);
    assert(state.last_command.sequence == 20U);
    assert(state.last_command.type == CMD_STOP);
    puts("[PASS] unknown command leaves state unchanged");

    result = dispatch_command(NULL, &stop);
    assert(result == -1);
    result = dispatch_command(&state, NULL);
    assert(result == -1);
    puts("[PASS] NULL arguments rejected");

    puts("Day19 command dispatch: all checks passed.");
    return 0;
}
