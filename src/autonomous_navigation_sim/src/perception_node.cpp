#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/string.hpp"

class PerceptionNode : public rclcpp::Node
{
public:
    PerceptionNode() : Node("perception_node")
    {   
        this->declare_parameter<std::string>("scan_topic", "/scan");
        std::string topic = this->get_parameter("scan_topic").as_string();

        scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            topic,
            10,
            std::bind(&PerceptionNode::scanCallback, this, std::placeholders::_1));

        safety_pub_ = this->create_publisher<std_msgs::msg::String>("/safety_state", 10);

        RCLCPP_INFO(this->get_logger(), "Perception node started. Listening to /scan...");
    }

private:
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr safety_pub_;

    float getMinDistanceInRange(
        const sensor_msgs::msg::LaserScan::SharedPtr msg,
        float start_angle,
        float end_angle)
    {
        float min_distance = std::numeric_limits<float>::infinity();

        for (size_t i = 0; i < msg->ranges.size(); ++i)
        {
            float angle = msg->angle_min + static_cast<float>(i) * msg->angle_increment;
            float distance = msg->ranges[i];

            if (angle >= start_angle && angle <= end_angle)
            {
                if (std::isfinite(distance) &&
                    distance >= msg->range_min &&
                    distance <= msg->range_max)
                {
                    min_distance = std::min(min_distance, distance);
                }
            }
        }

        return min_distance;
    }

    std::string classifySafety(float front_min, float left_min, float right_min)
    {
        if (front_min < 0.6f)
        {
            return "DANGER";
        }
        else if (front_min < 1.2f || left_min < 0.8f || right_min < 0.8f)
        {
            return "CAUTION";
        }
        else
        {
            return "SAFE";
        }
    }

    void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        constexpr float DEG_TO_RAD = 3.14159265f / 180.0f;

        float front_min = getMinDistanceInRange(msg, -30.0f * DEG_TO_RAD, 30.0f * DEG_TO_RAD);
        float left_min  = getMinDistanceInRange(msg, 30.0f * DEG_TO_RAD, 90.0f * DEG_TO_RAD);
        float right_min = getMinDistanceInRange(msg, -90.0f * DEG_TO_RAD, -30.0f * DEG_TO_RAD);

        std::string state = classifySafety(front_min, left_min, right_min);

        RCLCPP_INFO(
            this->get_logger(),
            "State: %s | Front: %.2f m | Left: %.2f m | Right: %.2f m",
            state.c_str(),
            front_min,
            left_min,
            right_min);

        std_msgs::msg::String out_msg;
        out_msg.data = state;
        safety_pub_->publish(out_msg);
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<PerceptionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}