#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SafetyMonitorNode : public rclcpp::Node
{
public:
    SafetyMonitorNode() : Node("safety_monitor_node")
    {
        safety_sub_ = this->create_subscription<std_msgs::msg::String>(
            "/safety_state",
            10,
            std::bind(&SafetyMonitorNode::safetyCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Safety monitor node started. Listening to /safety_state...");
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr safety_sub_;

    void safetyCallback(const std_msgs::msg::String::SharedPtr msg)
    {
        if (msg->data == "SAFE")
        {
            RCLCPP_INFO(this->get_logger(), "[SAFE] Path is clear.");
        }
        else if (msg->data == "CAUTION")
        {
            RCLCPP_WARN(this->get_logger(), "[CAUTION] Obstacle nearby. Slow down or prepare to turn.");
        }
        else if (msg->data == "DANGER")
        {
            RCLCPP_ERROR(this->get_logger(), "[DANGER] Obstacle too close. Stop movement.");
        }
        else
        {
            RCLCPP_WARN(this->get_logger(), "[UNKNOWN] Received unexpected safety state: %s", msg->data.c_str());
        }
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SafetyMonitorNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}