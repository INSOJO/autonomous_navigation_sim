#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/string.hpp>

#include <vector>
#include <string>
#include <limits>
#include <cmath>

class PerceptionNode : public rclcpp::Node {
public:
    PerceptionNode() : Node("perception_node") {
        subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan",
            10,
            std::bind(&PerceptionNode::scan_callback, this, std::placeholders::_1)
        );

        publisher_ = this->create_publisher<std_msgs::msg::String>(
            "/perception_state", 10
        );
    }

private:
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

    float compute_min(const std::vector<float>& ranges, int start, int end) {
        float min_val = std::numeric_limits<float>::infinity();

        for (int i = start; i < end; ++i) {
            if (std::isfinite(ranges[i]) && ranges[i] > 0.0f) {
                if (ranges[i] < min_val) {
                    min_val = ranges[i];
                }
            }
        }

        if (!std::isfinite(min_val)) {
            return 3.5f;
        }

        return min_val;
    }

    float compute_avg(const std::vector<float>& ranges, int start, int end) {
        float sum = 0.0f;
        int count = 0;

        for (int i = start; i < end; ++i) {
            if (std::isfinite(ranges[i]) && ranges[i] > 0.0f) {
                sum += ranges[i];
                ++count;
            }
        }

        if (count == 0) {
            return 3.5f;
        }

        return sum / count;
    }

    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        int size = static_cast<int>(msg->ranges.size());
        int third = size / 3;

        int left_start = 0;
        int left_end = third;

        int center_start = third;
        int center_end = 2 * third;

        int right_start = 2 * third;
        int right_end = size;

        float left_min = compute_min(msg->ranges, left_start, left_end);
        float center_min = compute_min(msg->ranges, center_start, center_end);
        float right_min = compute_min(msg->ranges, right_start, right_end);

        float left_avg = compute_avg(msg->ranges, left_start, left_end);
        float center_avg = compute_avg(msg->ranges, center_start, center_end);
        float right_avg = compute_avg(msg->ranges, right_start, right_end);

        const float blocked_threshold = 1.0f;
        const float caution_threshold = 0.7f;

        bool left_blocked = left_min < blocked_threshold;
        bool center_blocked = center_min < blocked_threshold;
        bool right_blocked = right_min < blocked_threshold;

        std::string best_direction = "STOP";

        if (!center_blocked) {
            best_direction = "FORWARD";
        } else if (!left_blocked && !right_blocked) {
            best_direction = (left_avg > right_avg) ? "LEFT" : "RIGHT";
        } else if (!left_blocked) {
            best_direction = "LEFT";
        } else if (!right_blocked) {
            best_direction = "RIGHT";
        } else if (left_min >= caution_threshold || right_min >= caution_threshold) {
            best_direction = (left_avg > right_avg) ? "LEFT" : "RIGHT";
        } else {
            best_direction = "STOP";
        }

        std_msgs::msg::String out;
        out.data =
            "left=" + std::to_string(left_min) +
            ";center=" + std::to_string(center_min) +
            ";right=" + std::to_string(right_min) +
            ";left_avg=" + std::to_string(left_avg) +
            ";center_avg=" + std::to_string(center_avg) +
            ";right_avg=" + std::to_string(right_avg) +
            ";best=" + best_direction;

        publisher_->publish(out);

        RCLCPP_INFO(
            this->get_logger(),
            "Perception | left_min=%.2f center_min=%.2f right_min=%.2f | left_avg=%.2f center_avg=%.2f right_avg=%.2f | best=%s",
            left_min, center_min, right_min,
            left_avg, center_avg, right_avg,
            best_direction.c_str()
        );
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PerceptionNode>());
    rclcpp::shutdown();
    return 0;
}