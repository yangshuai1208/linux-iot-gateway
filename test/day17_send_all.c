#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static bool SendAll(int socket_fd,
                    const uint8_t *data,
                    size_t length)
{
    size_t total_sent = 0U;

    if ((socket_fd < 0) ||
        ((data == NULL) && (length > 0U)))
    {
        return false;
    }

    while (total_sent < length)
    {
        ssize_t sent = send(socket_fd,
                            data + total_sent,
                            length - total_sent,
                            MSG_NOSIGNAL);

        /*
         * TODO：
         *
         * 1. sent > 0：
         *    累加total_sent。
         *
         * 2. sent == 0：
         *    为防止死循环，返回false。
         *
         * 3. sent < 0且errno == EINTR：
         *    不修改偏移量，重新调用send。
         *
         * 4. 其他错误：
         *    返回false。
         */
        if(sent>0)
        {
            total_sent+=(size_t)sent;
        }
        else if(sent==0)
        {
            return false;
        }
        else if(sent<0&&errno==EINTR)
        {
             continue;
        }
        else{
            return false;
        }
    }

    return true;
}

static bool ReceiveExactly(int socket_fd,
                           uint8_t *data,
                           size_t length)
{
    size_t total_received = 0U;

    while (total_received < length)
    {
        ssize_t received =
            recv(socket_fd,
                 data + total_received,
                 length - total_received,
                 0);

        if (received > 0)
        {
            total_received += (size_t)received;
        }
        else if ((received < 0) && (errno == EINTR))
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}

static void TestValidSend(void)
{
    static const uint8_t message[] =
        "HAND_OPEN\r\n";

    uint8_t received[sizeof(message) - 1U];
    int sockets[2];

    int result =
        socketpair(AF_UNIX,
                   SOCK_STREAM,
                   0,
                   sockets);

    assert(result == 0);

    bool sent =
        SendAll(sockets[0],
                message,
                sizeof(message) - 1U);

    assert(sent);
    int shutdown_result =
    shutdown(sockets[0], SHUT_WR);

assert(shutdown_result == 0);

    bool received_all =
        ReceiveExactly(sockets[1],
                       received,
                       sizeof(received));

    assert(received_all);
    assert(memcmp(message,
                  received,
                  sizeof(received)) == 0);

    close(sockets[0]);
    close(sockets[1]);
}

static void TestZeroLength(void)
{
    int sockets[2];

    int result =
        socketpair(AF_UNIX,
                   SOCK_STREAM,
                   0,
                   sockets);

    assert(result == 0);

    bool sent = SendAll(sockets[0], NULL, 0U);

    assert(sent);

    close(sockets[0]);
    close(sockets[1]);
}

static void TestInvalidArguments(void)
{
    static const uint8_t message[] = "TEST";

    assert(!SendAll(-1,
                    message,
                    sizeof(message) - 1U));

    assert(!SendAll(1, NULL, 1U));
}

static void TestClosedPeer(void)
{
    static const uint8_t message[] = "TEST";
    int sockets[2];

    int result =
        socketpair(AF_UNIX,
                   SOCK_STREAM,
                   0,
                   sockets);

    assert(result == 0);

    close(sockets[1]);

    bool sent =
        SendAll(sockets[0],
                message,
                sizeof(message) - 1U);

    assert(!sent);

    close(sockets[0]);
}

int main(void)
{
    TestValidSend();
    TestZeroLength();
    TestInvalidArguments();
    TestClosedPeer();

    printf("Day17 send_all tests passed\n");
    return 0;
}