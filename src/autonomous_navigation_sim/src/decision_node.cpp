#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include <sstream>
#include <map>
#include <string>

class DecisionNode : public rclcpp::Node {
public:
    DecisionNode() : Node("decision_node") {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/perception_state",
            10,
            std::bind(&DecisionNode::callback, this, std::placeholders::_1)
        );

        decision_publisher_ = this->create_publisher<std_msgs::msg::String>(
            "/navigation_decision", 10
        );

        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10
        );
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr decision_publisher_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;

    std::map<std::string, std::string> parse(const std::string & data) {
        std::map<std::string, std::string> result;
        std::stringstream ss(data);
        std::string item;

        while (getline(ss, item, ';')) {
            auto pos = item.find('=');
            if (pos != std::string::npos) {
                std::string key = item.substr(0, pos);
                std::string value = item.substr(pos + 1);
                result[key] = value;
            }
        }
        return result;
    }

    geometry_msgs::msg::Twist decisionToTwist(const std::string & decision) {
        geometry_msgs::msg::Twist cmd;

        if (decision == "FORWARD") {
            cmd.linear.x = 0.30;
            cmd.angular.z = 0.00;
        } else if (decision == "TURN_LEFT") {
            cmd.linear.x = 0.10;
            cmd.angular.z = 0.45;
        } else if (decision == "TURN_RIGHT") {
            cmd.linear.x = 0.10;
            cmd.angular.z = -0.45;
        } else {
            cmd.linear.x = 0.00;
            cmd.angular.z = 0.00;
        }

        return cmd;
    }

    void callback(const std_msgs::msg::String::SharedPtr msg) {
        auto data = parse(msg->data);

        std::string best = "STOP";
        if (data.count("best")) {
            best = data["best"];
        }

        std::string decision;
        if (best == "FORWARD") {
            decision = "FORWARD";
        } else if (best == "LEFT") {
            decision = "TURN_LEFT";
        } else if (best == "RIGHT") {
            decision = "TURN_RIGHT";
        } else {
            decision = "STOP";
        }

        std_msgs::msg::String decision_msg;
        decision_msg.data = decision;
        decision_publisher_->publish(decision_msg);

        auto cmd_msg = decisionToTwist(decision);
        cmd_vel_publisher_->publish(cmd_msg);

        RCLCPP_INFO(
            this->get_logger(),
            "Decision: %s | cmd_vel => linear.x=%.2f angular.z=%.2f",
            decision.c_str(),
            cmd_msg.linear.x,
            cmd_msg.angular.z
        );
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DecisionNode>());
    rclcpp::shutdown();
    return 0;
}