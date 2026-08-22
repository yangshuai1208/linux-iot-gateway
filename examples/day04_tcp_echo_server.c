#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 256U

static int send_all(int fd, const char *buffer, size_t length)
{
    size_t sent = 0U;

    while (sent < length)
    {
        ssize_t n = send(
            fd,
            buffer + sent,
            length - sent,
            0
        );

        if (n > 0)
        {
            sent += (size_t)n;
        }
        else if (n < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    int reuse = 1;
    int exit_status = 1;

    struct sockaddr_in server_addr = {0};
    char buffer[BUFFER_SIZE];

    /* 防止客户端突然断开时send触发SIGPIPE终止程序 */
    (void)signal(SIGPIPE, SIG_IGN);

    /* 1. 创建TCP套接字 */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        goto cleanup;
    }

    /* 2. 允许快速重新绑定端口 */
    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) < 0)
    {
        perror("setsockopt");
        goto cleanup;
    }

    /* 3. 填写服务器地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 4. 绑定IP和端口 */
    if (bind(
            server_fd,
            (const struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0)
    {
        perror("bind");
        goto cleanup;
    }

    /* 5. 进入监听状态 */
    if (listen(server_fd, LISTEN_BACKLOG) < 0)
    {
        perror("listen");
        goto cleanup;
    }

    printf("server listening on 0.0.0.0:%d\n",
           SERVER_PORT);

    /* 6. 等待客户端连接 */
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0)
    {
        perror("accept");
        goto cleanup;
    }

    puts("client connected");

    /* 7. 循环接收和回显 */
    for (;;)
    {
        ssize_t received = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1U,
            0
        );

        if (received > 0)
        {
            /* recv不会自动添加字符串结束符 */
            buffer[received] = '\0';

            printf(
                "received %zd bytes: %s",
                received,
                buffer
            );

            if (send_all(
                    client_fd,
                    buffer,
                    (size_t)received
                ) < 0)
            {
                perror("send");
                goto cleanup;
            }
        }
        else if (received == 0)
        {
            puts("client closed the connection");
            exit_status = 0;
            break;
        }
        else if (errno == EINTR)
        {
            /* 被信号中断，重新recv */
            continue;
        }
        else
        {
            perror("recv");
            goto cleanup;
        }
    }

cleanup:

    if (client_fd >= 0)
    {
        (void)close(client_fd);
    }

    if (server_fd >= 0)
    {
        (void)close(server_fd);
    }

    return exit_status;
}