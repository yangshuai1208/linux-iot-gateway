#include <cstdint>
#include <functional>
#include <memory>
#include <string>

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
        /* 1. 声明ROS2参数 */
        this->declare_parameter<std::string>(
            "uart_port",
            "/dev/ttyUSB0");

        this->declare_parameter<int64_t>(
            "baud_rate",
            115200);


        /* 2. 获取参数 */
        uart_port_ =
            this->get_parameter("uart_port").as_string();

        baud_rate_ =
            this->get_parameter("baud_rate").as_int();


        /* 3. 订阅控制命令 */
        command_sub_ =
            this->create_subscription<std_msgs::msg::String>(
                "/hand_command",
                10,
                std::bind(
                    &HandBridgeNode::command_callback,
                    this,
                    std::placeholders::_1));


        /* 4. 创建状态查询Service */
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
            "Hand bridge node started");


        RCLCPP_INFO(
            this->get_logger(),
            "UART port=%s, baud=%ld",
            uart_port_.c_str(),
            static_cast<long>(baud_rate_));
    }


private:

    /* 判断是否合法命令 */
    bool valid_command(const std::string &cmd)
    {
        return cmd == "OPEN" ||
               cmd == "GRAB" ||
               cmd == "RELEASE" ||
               cmd == "STOP";
    }


    /* /hand_command 回调 */
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


        RCLCPP_INFO(
            this->get_logger(),
            "Received command: %s",
            current_state_.c_str());
    }


    /* /hand_status Service回调 */
    void status_callback(
        const std::shared_ptr<
            std_srvs::srv::Trigger::Request> request,

        std::shared_ptr<
            std_srvs::srv::Trigger::Response> response)
    {
        (void)request;


        response->success = true;

        response->message =
            "current state: " + current_state_;


        RCLCPP_INFO(
            this->get_logger(),
            "Status requested: %s",
            current_state_.c_str());
    }


    /* ROS2参数 */
    std::string uart_port_;

    int64_t baud_rate_;


    /* 当前节点记录的最近命令 */
    std::string current_state_;


    /* Topic Subscriber */
    rclcpp::Subscription<
        std_msgs::msg::String>::SharedPtr command_sub_;


    /* Service */
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