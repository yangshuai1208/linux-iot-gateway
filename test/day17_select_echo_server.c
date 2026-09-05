#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 9090U
#define SERVER_BACKLOG 8
#define BUFFER_SIZE 256U

static bool SendAll(int socket_fd,
                    const uint8_t *data,
                    size_t length)
{
    size_t total_sent = 0U;

    while (total_sent < length)
    {
        ssize_t sent =
            send(socket_fd,
                 data + total_sent,
                 length - total_sent,
                 MSG_NOSIGNAL);

        if (sent > 0)
        {
            total_sent += (size_t)sent;
        }
        else if ((sent < 0) && (errno == EINTR))
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

static int CreateListenSocket(void)
{
    int listen_fd =
        socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd < 0)
    {
        perror("socket");
        return -1;
    }

    int reuse_address = 1;

    if (setsockopt(listen_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuse_address,
                   sizeof(reuse_address)) < 0)
    {
        perror("setsockopt");
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(listen_fd,
             (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, SERVER_BACKLOG) < 0)
    {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

static void RemoveDescriptor(int descriptor,
                             fd_set *master_set,
                             int *max_fd)
{
    if ((descriptor < 0) ||
        (master_set == NULL) ||
        (max_fd == NULL))
    {
        return;
    }

    /*
     * TODO 4：
     * 1. 使用FD_CLR从master_set中删除descriptor。
     * 2. 调用close关闭descriptor。
     * 3. 如果descriptor等于max_fd，从当前max_fd开始
     *    向下寻找仍然存在于master_set中的最大描述符。
     */
    FD_CLR(descriptor,master_set);
    close(descriptor);
    if(descriptor==*max_fd)
    {
        while (*max_fd>0&&!FD_ISSET(*max_fd,master_set))
        {
            (*max_fd)--;
        }
        
    }




}

int main(void)
{
    int listen_fd = CreateListenSocket();

    if (listen_fd < 0)
    {
        return 1;
    }

    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(listen_fd, &master_set);

    int max_fd = listen_fd;

    printf("Server listening on 0.0.0.0:%u\n",
           SERVER_PORT);

    while (true)
    {
        fd_set read_set;

        /*
         * TODO 1：
         * 将master_set复制给read_set，
         * 防止select修改长期保存的master_set。
         */
        read_set=master_set;

        /*
         * TODO 2：
         * 调用select。
         * 第一个参数应覆盖0到max_fd。
         * 只监听read_set，不设置写集合和异常集合。
         * timeout传NULL，表示一直等待。
         */
    
        int ready_count = select(max_fd+1,&read_set,NULL,NULL,NULL);

        if (ready_count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("select");
            break;
        }

        for (int descriptor = 0;
             (descriptor <= max_fd) &&
             (ready_count > 0);
             descriptor++)
        {
            if (!FD_ISSET(descriptor, &read_set))
            {
                continue;
            }

            ready_count--;

            if (descriptor == listen_fd)
            {
                int client_fd =
                    accept(listen_fd, NULL, NULL);

                if (client_fd < 0)
                {
                    if (errno != EINTR)
                    {
                        perror("accept");
                    }

                    continue;
                }

                if (client_fd >= FD_SETSIZE)
                {
                    fprintf(stderr,
                            "client fd exceeds FD_SETSIZE\n");
                    close(client_fd);
                    continue;
                }

                /*
                 * TODO 3：
                 * 1. 将client_fd加入master_set。
                 * 2. 如果client_fd大于max_fd，更新max_fd。
                 */
                FD_SET(client_fd,&master_set);
                if(client_fd>max_fd)
                {
                    max_fd=client_fd;
                }
                printf("Client connected: fd=%d\n",
                       client_fd);
            }
            else
            {
                uint8_t buffer[BUFFER_SIZE];

                ssize_t received =
                    recv(descriptor,
                         buffer,
                         sizeof(buffer),
                         0);

                if (received > 0)
                {
                    printf("Received %zd bytes from fd=%d\n",
                           received,
                           descriptor);

                    if (!SendAll(descriptor,
                                 buffer,
                                 (size_t)received))
                    {
                        perror("send");
                        RemoveDescriptor(descriptor,
                                         &master_set,
                                         &max_fd);
                    }
                }
                else if (received == 0)
                {
                    printf("Client disconnected: fd=%d\n",
                           descriptor);

                    RemoveDescriptor(descriptor,
                                     &master_set,
                                     &max_fd);
                }
                else if (errno != EINTR)
                {
                    perror("recv");

                    RemoveDescriptor(descriptor,
                                     &master_set,
                                     &max_fd);
                }
            }
        }
    }

    for (int descriptor = 0;
         descriptor <= max_fd;
         descriptor++)
    {
        if (FD_ISSET(descriptor, &master_set))
        {
            close(descriptor);
        }
    }

    return 0;
}