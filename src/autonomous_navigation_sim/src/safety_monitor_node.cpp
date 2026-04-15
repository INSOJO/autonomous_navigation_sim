#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SafetyMonitorNode : public rclcpp::Node
{
public:
    SafetyMonitorNode() : Node("safety_monitor_node")
    {
        perception_sub_ = this->create_subscription<std_msgs::msg::String>(
            "/perception_state",
            10,
            std::bind(&SafetyMonitorNode::perceptionCallback, this, std::placeholders::_1));

        decision_sub_ = this->create_subscription<std_msgs::msg::String>(
            "/navigation_decision",
            10,
            std::bind(&SafetyMonitorNode::decisionCallback, this, std::placeholders::_1));

        RCLCPP_INFO(
            this->get_logger(),
            "Safety monitor node started. Listening to /perception_state and /navigation_decision...");
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr perception_sub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr decision_sub_;

    std::string latest_perception_;
    std::string latest_decision_;

    void perceptionCallback(const std_msgs::msg::String::SharedPtr msg)
    {
        latest_perception_ = msg->data;
    }

    void decisionCallback(const std_msgs::msg::String::SharedPtr msg)
    {
        latest_decision_ = msg->data;
        printStatus();
    }

    void printStatus()
    {
        if (latest_perception_.empty() || latest_decision_.empty()) {
            return;
        }

        if (latest_decision_ == "STOP") {
            RCLCPP_ERROR(
                this->get_logger(),
                "Perception: [%s] | Decision: [%s]",
                latest_perception_.c_str(),
                latest_decision_.c_str());
        } else if (latest_decision_ == "TURN_LEFT" || latest_decision_ == "TURN_RIGHT") {
            RCLCPP_WARN(
                this->get_logger(),
                "Perception: [%s] | Decision: [%s]",
                latest_perception_.c_str(),
                latest_decision_.c_str());
        } else {
            RCLCPP_INFO(
                this->get_logger(),
                "Perception: [%s] | Decision: [%s]",
                latest_perception_.c_str(),
                latest_decision_.c_str());
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