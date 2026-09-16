#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class HandBridgeNode : public rclcpp::Node
{
public:
    HandBridgeNode()
        : Node("hand_bridge_node")
    {
        subscriber_ =
            this->create_subscription<std_msgs::msg::String>(
                "/hand_command",
                10,
                std::bind(
                    &HandBridgeNode::command_callback,
                    this,
                    std::placeholders::_1));

        RCLCPP_INFO(
            this->get_logger(),
            "Hand bridge node started");
    }

private:
    void command_callback(
        const std_msgs::msg::String::SharedPtr msg)
    {
        const std::string &command = msg->data;

        if (command == "OPEN" ||
            command == "GRAB" ||
            command == "RELEASE" ||
            command == "STOP")
        {
            RCLCPP_INFO(
                this->get_logger(),
                "Receive hand command: %s",
                command.c_str());
        }
        else
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Unknown command: %s",
                command.c_str());
        }
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(
        std::make_shared<HandBridgeNode>());

    rclcpp::shutdown();

    return 0;
}