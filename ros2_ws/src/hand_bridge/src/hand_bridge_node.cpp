
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "hand_bridge/ack_parser.hpp"
#include "hand_bridge/retry_policy.hpp"

class HandBridgeNode : public rclcpp::Node
{
public:
    HandBridgeNode()
        : Node("hand_bridge_node")
    {
        this->declare_parameter<std::string>(
            "uart_port", "/dev/ttyUSB0");

        this->declare_parameter<int64_t>(
            "baud_rate", 115200);

        uart_port_ =
            this->get_parameter("uart_port").as_string();

        baud_rate_ =
            this->get_parameter("baud_rate").as_int();

        if (open_uart())
        {
            tx_thread_ = std::thread(
                &HandBridgeNode::uart_tx_worker, this);

            rx_thread_ = std::thread(
                &HandBridgeNode::uart_rx_worker, this);

            retry_thread_ = std::thread(
                &HandBridgeNode::retry_worker, this);
        }
        else
        {
            RCLCPP_WARN(
                this->get_logger(),
                "UART unavailable, ROS node continues");
        }

        command_sub_ =
            this->create_subscription<std_msgs::msg::String>(
                "/hand_command",
                10,
                std::bind(
                    &HandBridgeNode::command_callback,
                    this,
                    std::placeholders::_1));

        status_service_ =
            this->create_service<std_srvs::srv::Trigger>(
                "/hand_status",
                std::bind(
                    &HandBridgeNode::status_callback,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2));

        RCLCPP_INFO(
            this->get_logger(),
            "Hand bridge started, port=%s, baud=%ld",
            uart_port_.c_str(),
            static_cast<long>(baud_rate_));
    }

    ~HandBridgeNode() override
    {
        retry_stop_.store(true);
        rx_stop_.store(true);

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            stop_worker_ = true;
            command_queue_.clear();
        }

        queue_cv_.notify_all();

        if (retry_thread_.joinable())
        {
            retry_thread_.join();
        }

        if (tx_thread_.joinable())
        {
            tx_thread_.join();
        }

        if (rx_thread_.joinable())
        {
            rx_thread_.join();
        }

        if (uart_fd_ >= 0)
        {
            ::close(uart_fd_);
            uart_fd_ = -1;
        }
    }

private:
    static bool valid_command(const std::string &cmd)
    {
        return cmd == "OPEN" ||
               cmd == "GRAB" ||
               cmd == "RELEASE" ||
               cmd == "STOP";
    }

    static std::string map_to_uart_command(
        const std::string &cmd,
        std::uint32_t seq)
    {
        std::string action;

        if (cmd == "OPEN")
        {
            action = "HAND_OPEN";
        }
        else if (cmd == "GRAB")
        {
            action = "HAND_GRAB";
        }
        else if (cmd == "RELEASE")
        {
            action = "HAND_RELEASE";
        }
        else if (cmd == "STOP")
        {
            action = "HAND_STOP";
        }
        else
        {
            return "";
        }

        return "SEQ:" +
               std::to_string(seq) +
               " CMD:" +
               action +
               "\r\n";
    }

    static const char *request_state_to_string(
        hand_bridge::RequestState state)
    {
        using hand_bridge::RequestState;

        switch (state)
        {
        case RequestState::QUEUED:
            return "QUEUED";

        case RequestState::WAITING_ACK:
            return "WAITING_ACK";

        case RequestState::EXECUTING:
            return "EXECUTING";

        case RequestState::RETRY_QUEUED:
            return "RETRY_QUEUED";

        case RequestState::SUCCEEDED:
            return "SUCCEEDED";

        case RequestState::PREEMPTED:
            return "PREEMPTED";

        case RequestState::BUSY:
            return "BUSY";

        case RequestState::ERROR:
            return "ERROR";

        case RequestState::SUPERSEDED:
            return "SUPERSEDED";

        case RequestState::RESULT_UNKNOWN:
            return "RESULT_UNKNOWN";

        default:
            return "UNKNOWN";
        }
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

        if (uart_fd_ < 0)
        {
            RCLCPP_WARN(
                this->get_logger(),
                "UART unavailable, command not sent");

            return;
        }

        const std::uint32_t seq =
            next_seq_.fetch_add(
                1U,
                std::memory_order_relaxed);

        if (seq == 0U)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "SEQ exhausted");

            return;
        }

        const bool is_stop =
            (msg->data == "STOP");

        const std::string frame =
            map_to_uart_command(msg->data, seq);

        if (frame.empty())
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(ack_mutex_);

            if (is_stop)
            {
                retry_policy_.cancel_non_stop();
            }

            if (!retry_policy_.start(
                    seq,
                    frame,
                    is_stop))
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "Register request failed: seq=%u",
                    static_cast<unsigned>(seq));

                return;
            }

            latest_requested_seq_ = seq;
        }

        if (!enqueue_uart_command(
                {seq, frame},
                is_stop))
        {
            std::lock_guard<std::mutex> lock(ack_mutex_);

            retry_policy_.mark_send_failed(seq);

            return;
        }

        RCLCPP_INFO(
            this->get_logger(),
            "Queued command=%s seq=%u",
            msg->data.c_str(),
            static_cast<unsigned>(seq));
    }

    void status_callback(
        const std::shared_ptr<
            std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<
            std_srvs::srv::Trigger::Response> response)
    {
        (void)request;

        std::lock_guard<std::mutex> lock(ack_mutex_);

        const std::uint32_t seq =
            latest_requested_seq_;

        if (seq == 0)
        {
            response->success = false;
            response->message = "No command submitted";
            return;
        }

        const auto *item =
            retry_policy_.find(seq);

        if (item == nullptr)
        {
            response->success = false;
            response->message = "Request not found";
            return;
        }

        response->success =
            item->state ==
            hand_bridge::RequestState::SUCCEEDED;

        response->message =
            "SEQ:" +
            std::to_string(seq) +
            " STATUS:" +
            request_state_to_string(item->state);
    }

    bool open_uart()
    {
        if (baud_rate_ != 115200)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Only 115200 baud is supported");

            return false;
        }

        uart_fd_ = ::open(
            uart_port_.c_str(),
            O_RDWR | O_NOCTTY);

        if (uart_fd_ < 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Open UART failed: %s",
                std::strerror(errno));

            return false;
        }

        struct termios tty {};

        if (tcgetattr(uart_fd_, &tty) != 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "tcgetattr failed: %s",
                std::strerror(errno));

            ::close(uart_fd_);
            uart_fd_ = -1;

            return false;
        }

        cfmakeraw(&tty);

        if (cfsetispeed(&tty, B115200) != 0 ||
            cfsetospeed(&tty, B115200) != 0)
        {
            ::close(uart_fd_);
            uart_fd_ = -1;

            return false;
        }

        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;

        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        tty.c_cflag |= CREAD | CLOCAL;

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

        tcflush(uart_fd_, TCIOFLUSH);

        RCLCPP_INFO(
            this->get_logger(),
            "UART opened: %s",
            uart_port_.c_str());

        return true;
    }

    bool write_uart(const std::string &data)
    {
        if (uart_fd_ < 0)
        {
            return false;
        }

        std::size_t total_written = 0;

        while (total_written < data.size())
        {
            const ssize_t ret = ::write(
                uart_fd_,
                data.data() + total_written,
                data.size() - total_written);

            if (ret > 0)
            {
                total_written +=
                    static_cast<std::size_t>(ret);

                continue;
            }

            if (ret < 0 && errno == EINTR)
            {
                continue;
            }

            if (ret == 0)
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "UART write returned zero");

                return false;
            }

            RCLCPP_ERROR(
                this->get_logger(),
                "UART write failed: %s",
                std::strerror(errno));

            return false;
        }

        return true;
    }

    bool enqueue_uart_command(
        const hand_bridge::RetryFrame &frame,
        bool high_priority = false)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            if (stop_worker_)
            {
                return false;
            }

            if (high_priority)
            {
                command_queue_.push_front(frame);
            }
            else
            {
                command_queue_.push_back(frame);
            }
        }

        queue_cv_.notify_one();

        return true;
    }

    void uart_tx_worker()
    {
        using Clock = hand_bridge::RetryPolicy::Clock;

        while (true)
        {
            hand_bridge::RetryFrame frame{};

            {
                std::unique_lock<std::mutex> lock(
                    queue_mutex_);

                queue_cv_.wait(
                    lock,
                    [this]()
                    {
                        return stop_worker_ ||
                               !command_queue_.empty();
                    });

                if (stop_worker_)
                {
                    break;
                }

                frame = command_queue_.front();
                command_queue_.pop_front();
            }

            bool sent = false;

            {
                std::lock_guard<std::mutex> lock(
                    ack_mutex_);

                if (!retry_policy_.should_transmit(
                        frame.seq))
                {
                    continue;
                }

                sent = write_uart(frame.frame);

                if (sent)
                {
                    retry_policy_.mark_sent(
                        frame.seq,
                        Clock::now());
                }
                else
                {
                    retry_policy_.mark_send_failed(
                        frame.seq);
                }
            }

            if (sent)
            {
                RCLCPP_INFO(
                    this->get_logger(),
                    "UART TX seq=%u",
                    static_cast<unsigned>(frame.seq));
            }
            else
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "UART TX failed seq=%u",
                    static_cast<unsigned>(frame.seq));
            }
        }

        RCLCPP_INFO(
            this->get_logger(),
            "UART TX worker stopped");
    }

    void uart_rx_worker()
    {
        char buffer[128];

        std::string rx_line;

        bool discard_line = false;

        while (!rx_stop_.load())
        {
            const ssize_t ret = ::read(
                uart_fd_,
                buffer,
                sizeof(buffer));

            if (ret == 0)
            {
                continue;
            }

            if (ret < 0)
            {
                if (errno == EINTR ||
                    errno == EAGAIN)
                {
                    continue;
                }

                RCLCPP_ERROR(
                    this->get_logger(),
                    "UART read failed: %s",
                    std::strerror(errno));

                break;
            }

            for (ssize_t i = 0; i < ret; ++i)
            {
                const char ch = buffer[i];

                if (ch == '\n')
                {
                    if (!discard_line &&
                        !rx_line.empty())
                    {
                        std::uint32_t seq = 0;

                        hand_bridge::AckStatus status =
                            hand_bridge::AckStatus::ERROR;

                        if (hand_bridge::parse_ack(
                                rx_line + "\n",
                                seq,
                                status))
                        {
                            bool accepted = false;

                            {
                                std::lock_guard<std::mutex> lock(
                                    ack_mutex_);

                                accepted =
                                    retry_policy_.on_ack(
                                        seq,
                                        status,
                                        hand_bridge::RetryPolicy::Clock::now());
                            }

                            if (accepted)
                            {
                                RCLCPP_INFO(
                                    this->get_logger(),
                                    "UART ACK seq=%u status=%s",
                                    static_cast<unsigned>(seq),
                                    hand_bridge::ack_status_to_string(
                                        status));
                            }
                            else
                            {
                                RCLCPP_WARN(
                                    this->get_logger(),
                                    "Unknown or stale ACK seq=%u",
                                    static_cast<unsigned>(seq));
                            }
                        }
                        else
                        {
                            RCLCPP_WARN(
                                this->get_logger(),
                                "Invalid UART line: %s",
                                rx_line.c_str());
                        }
                    }

                    rx_line.clear();
                    discard_line = false;

                    continue;
                }

                if (!discard_line)
                {
                    if (rx_line.size() < 95U)
                    {
                        rx_line.push_back(ch);
                    }
                    else
                    {
                        rx_line.clear();
                        discard_line = true;
                    }
                }
            }
        }

        RCLCPP_INFO(
            this->get_logger(),
            "UART RX worker stopped");
    }

    void retry_worker()
    {
        using Clock = hand_bridge::RetryPolicy::Clock;

        while (!retry_stop_.load())
        {
            std::vector<hand_bridge::RetryFrame> frames;

            {
                std::lock_guard<std::mutex> lock(
                    ack_mutex_);

                frames = retry_policy_.poll(
                    Clock::now());
            }

            for (const auto &frame : frames)
            {
                if (retry_stop_.load())
                {
                    break;
                }

                RCLCPP_WARN(
                    this->get_logger(),
                    "Retry queued seq=%u",
                    static_cast<unsigned>(frame.seq));

                enqueue_uart_command(frame);
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(50));
        }

        RCLCPP_INFO(
            this->get_logger(),
            "Retry worker stopped");
    }

    std::string uart_port_;
    int64_t baud_rate_ = 115200;

    int uart_fd_ = -1;

    std::atomic<std::uint32_t> next_seq_{1};

    std::uint32_t latest_requested_seq_ = 0;

    std::deque<hand_bridge::RetryFrame> command_queue_;

    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    bool stop_worker_ = false;

    std::thread tx_thread_;
    std::thread rx_thread_;
    std::thread retry_thread_;

    std::atomic<bool> rx_stop_{false};
    std::atomic<bool> retry_stop_{false};

    std::mutex ack_mutex_;

    hand_bridge::RetryPolicy retry_policy_;

    rclcpp::Subscription<
        std_msgs::msg::String>::SharedPtr command_sub_;

    rclcpp::Service<
        std_srvs::srv::Trigger>::SharedPtr status_service_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node =
        std::make_shared<HandBridgeNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
