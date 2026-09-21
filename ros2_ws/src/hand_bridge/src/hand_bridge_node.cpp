#include <cerrno>
#include <condition_variable>
#include <cstring>
#include <cstdint>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"


class HandBridgeNode : public rclcpp::Node
{
public:
    HandBridgeNode()
        : Node("hand_bridge_node"),
          current_state_("STOP")
    {
        /* 1. 声明参数 */
        this->declare_parameter<std::string>(
            "uart_port",
            "/dev/ttyUSB0");

        this->declare_parameter<int64_t>(
            "baud_rate",
            115200);

        /* 2. 获取参数 */
        uart_port_ =
            this->get_parameter(
                "uart_port").as_string();

        baud_rate_ =
            this->get_parameter(
                "baud_rate").as_int();

        /*
         * 3. 打开UART。
         * UART失败不让整个ROS节点退出。
         */
        if (!open_uart())
        {
            RCLCPP_WARN(
                this->get_logger(),
                "UART is unavailable, "
                "ROS node will continue running");
        }
        else
        {
            tx_thread_ =
                std::thread(
                    &HandBridgeNode::uart_tx_worker,
                    this);
        }

        /* 4. Topic订阅 */
        command_sub_ =
            this->create_subscription<
                std_msgs::msg::String>(
                "/hand_command",
                10,
                std::bind(
                    &HandBridgeNode::command_callback,
                    this,
                    std::placeholders::_1));

        /* 5. Service */
        status_service_ =
            this->create_service<
                std_srvs::srv::Trigger>(
                "/hand_status",
                std::bind(
                    &HandBridgeNode::status_callback,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2));

        RCLCPP_INFO(
            this->get_logger(),
            "Hand bridge node started");

        RCLCPP_INFO(
            this->get_logger(),
            "UART port=%s, baud=%ld",
            uart_port_.c_str(),
            static_cast<long>(baud_rate_));
    }


    ~HandBridgeNode()
    {
        /*
         * 通知TX线程退出
         */
        {
            std::lock_guard<std::mutex> lock(
                queue_mutex_);

            stop_worker_ = true;
        }

        queue_cv_.notify_all();

        /*
         * 等待线程结束
         */
        if (tx_thread_.joinable())
        {
            tx_thread_.join();
        }

        /*
         * 关闭文件描述符
         */
        if (uart_fd_ >= 0)
        {
            ::close(uart_fd_);
            uart_fd_ = -1;
        }
    }


private:

    bool valid_command(
        const std::string &cmd)
    {
        return cmd == "OPEN" ||
               cmd == "GRAB" ||
               cmd == "RELEASE" ||
               cmd == "STOP";
    }


    std::string map_to_uart_command(
        const std::string &cmd)
    {
        if (cmd == "OPEN")
        {
            return "HAND_OPEN\r\n";
        }

        if (cmd == "GRAB")
        {
            return "HAND_GRAB\r\n";
        }

        if (cmd == "RELEASE")
        {
            return "HAND_RELEASE\r\n";
        }

        if (cmd == "STOP")
        {
            return "HAND_STOP\r\n";
        }

        return "";
    }


    void command_callback(
        const std_msgs::msg::String::SharedPtr msg)
    {
        if (!valid_command(msg->data))
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Invalid command: %s",
                msg->data.c_str());

            return;
        }

        current_state_ = msg->data;

        const std::string uart_command =
            map_to_uart_command(msg->data);

        RCLCPP_INFO(
            this->get_logger(),
            "Received command: %s",
            current_state_.c_str());

        RCLCPP_INFO(
            this->get_logger(),
            "Mapped UART command: %s",
            uart_command.c_str());

        /*
         * callback只入队，不直接write。
         */
        if (uart_fd_ >= 0)
        {
            enqueue_uart_command(
                uart_command);
        }
        else
        {
            RCLCPP_WARN(
                this->get_logger(),
                "UART unavailable, command not sent");
        }
    }


    void status_callback(
        const std::shared_ptr<
            std_srvs::srv::Trigger::Request> request,

        std::shared_ptr<
            std_srvs::srv::Trigger::Response> response)
    {
        (void)request;

        response->success = true;

        response->message =
            "current state: " +
            current_state_;

        RCLCPP_INFO(
            this->get_logger(),
            "Status requested: %s",
            current_state_.c_str());
    }


    bool open_uart()
    {
        uart_fd_ =
            ::open(
                uart_port_.c_str(),
                O_RDWR | O_NOCTTY);

        if (uart_fd_ < 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Failed to open UART %s: %s",
                uart_port_.c_str(),
                std::strerror(errno));

            return false;
        }

        struct termios tty {};

        if (tcgetattr(
                uart_fd_,
                &tty) != 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "tcgetattr failed: %s",
                std::strerror(errno));

            ::close(uart_fd_);
            uart_fd_ = -1;

            return false;
        }

        /*
         * 115200
         */
        cfsetispeed(
            &tty,
            B115200);

        cfsetospeed(
            &tty,
            B115200);

        /*
         * 8位数据位
         */
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;

        /*
         * N：无校验
         */
        tty.c_cflag &= ~PARENB;

        /*
         * 1位停止位
         */
        tty.c_cflag &= ~CSTOPB;

        /*
         * 关闭硬件流控
         */
        tty.c_cflag &= ~CRTSCTS;

        /*
         * 允许接收
         * 忽略modem控制线
         */
        tty.c_cflag |=
            CREAD | CLOCAL;

        /*
         * raw模式
         */
        tty.c_lflag &=
            ~(ICANON |
              ECHO |
              ECHOE |
              ISIG);

        tty.c_iflag &=
            ~(IXON |
              IXOFF |
              IXANY);

        tty.c_oflag &=
            ~OPOST;

        /*
         * RX超时配置
         */
        tty.c_cc[VMIN] = 0;

        tty.c_cc[VTIME] = 5;

        if (tcsetattr(
                uart_fd_,
                TCSANOW,
                &tty) != 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "tcsetattr failed: %s",
                std::strerror(errno));

            ::close(uart_fd_);
            uart_fd_ = -1;

            return false;
        }

        tcflush(
            uart_fd_,
            TCIOFLUSH);

        RCLCPP_INFO(
            this->get_logger(),
            "UART opened: %s, 115200 8N1",
            uart_port_.c_str());

        return true;
    }


    bool write_uart(
        const std::string &data)
    {
        if (uart_fd_ < 0)
        {
            return false;
        }

        std::size_t total_written = 0;

        while (
            total_written <
            data.size())
        {
            ssize_t ret =
                ::write(
                    uart_fd_,
                    data.data() +
                        total_written,

                    data.size() -
                        total_written);

            if (ret > 0)
            {
                total_written +=
                    static_cast<
                        std::size_t>(ret);

                continue;
            }

            if (ret < 0 &&
                errno == EINTR)
            {
                continue;
            }

            RCLCPP_ERROR(
                this->get_logger(),
                "UART write failed: %s",
                std::strerror(errno));

            return false;
        }

        return true;
    }


    void enqueue_uart_command(
        const std::string &command)
    {
        {
            std::lock_guard<
                std::mutex> lock(
                    queue_mutex_);

            command_queue_.push(
                command);
        }

        queue_cv_.notify_one();
    }


    void uart_tx_worker()
    {
        while (true)
        {
            std::string command;

            {
                std::unique_lock<
                    std::mutex> lock(
                        queue_mutex_);

                queue_cv_.wait(
                    lock,
                    [this]()
                    {
                        return
                            stop_worker_ ||
                            !command_queue_.empty();
                    });

                if (stop_worker_ &&
                    command_queue_.empty())
                {
                    break;
                }

                command =
                    command_queue_.front();

                command_queue_.pop();
            }

            /*
             * write放在锁外面。
             */
            if (write_uart(command))
            {
                RCLCPP_INFO(
                    this->get_logger(),
                    "UART TX: %s",
                    command.c_str());
            }
            else
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "UART TX failed");
            }
        }
    }


    /* ROS参数 */
    std::string uart_port_;

    int64_t baud_rate_;


    /* 最近收到的ROS命令 */
    std::string current_state_;


    /* UART */
    int uart_fd_ = -1;


    /* TX线程安全队列 */
    std::queue<std::string>
        command_queue_;

    std::mutex queue_mutex_;

    std::condition_variable
        queue_cv_;

    bool stop_worker_ = false;

    std::thread tx_thread_;


    /* ROS Topic */
    rclcpp::Subscription<
        std_msgs::msg::String>::SharedPtr
        command_sub_;


    /* ROS Service */
    rclcpp::Service<
        std_srvs::srv::Trigger>::SharedPtr
        status_service_;
};


int main(
    int argc,
    char **argv)
{
    rclcpp::init(
        argc,
        argv);

    auto node =
        std::make_shared<
            HandBridgeNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}