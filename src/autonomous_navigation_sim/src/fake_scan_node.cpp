#include <chrono>
#include <cmath>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

using namespace std::chrono_literals;

class FakeScanNode : public rclcpp::Node
{
public:
    FakeScanNode() : Node("fake_scan_node"), step_(0)
    {
        scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>("/scan", 10);
        timer_ = this->create_wall_timer(
            1000ms, std::bind(&FakeScanNode::publishFakeScan, this));

        RCLCPP_INFO(this->get_logger(), "Fake scan node started. Publishing to /scan...");
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int step_;

    void publishFakeScan()
    {
        sensor_msgs::msg::LaserScan scan_msg;

        scan_msg.header.stamp = this->get_clock()->now();
        scan_msg.header.frame_id = "laser_frame";

        scan_msg.angle_min = -1.5708f;   // -90 deg
        scan_msg.angle_max = 1.5708f;    // +90 deg
        scan_msg.angle_increment = 3.1416f / 180.0f;  // 1 deg
        scan_msg.range_min = 0.12f;
        scan_msg.range_max = 10.0f;

        int num_readings = static_cast<int>(
            (scan_msg.angle_max - scan_msg.angle_min) / scan_msg.angle_increment) + 1;

        scan_msg.ranges.assign(num_readings, 3.0f);  // default = clear path

        // Create changing obstacle scenarios
        if (step_ % 4 == 0)
        {
            // Safe: everything far away
            for (auto &range : scan_msg.ranges) {
                range = 3.0f;
            }
            RCLCPP_INFO(this->get_logger(), "Scenario: SAFE");
        }
        else if (step_ % 4 == 1)
        {
            // Caution: obstacle in front around 1.0 m
            for (int i = 80; i <= 100; ++i) {
                scan_msg.ranges[i] = 1.0f;
            }
            RCLCPP_INFO(this->get_logger(), "Scenario: CAUTION");
        }
        else if (step_ % 4 == 2)
        {
            // Danger: obstacle very close in front
            for (int i = 80; i <= 100; ++i) {
                scan_msg.ranges[i] = 0.4f;
            }
            RCLCPP_INFO(this->get_logger(), "Scenario: DANGER");
        }
        else
        {
            // Caution: obstacle on left side
            for (int i = 120; i <= 150; ++i) {
                scan_msg.ranges[i] = 0.7f;
            }
            RCLCPP_INFO(this->get_logger(), "Scenario: LEFT OBSTACLE");
        }

        scan_pub_->publish(scan_msg);
        step_++;
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<FakeScanNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}