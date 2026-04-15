#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

class FakeScanNode : public rclcpp::Node {
public:
    FakeScanNode() : Node("fake_scan_node"), scenario_(0) {
        publisher_ = this->create_publisher<sensor_msgs::msg::LaserScan>("/scan", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&FakeScanNode::publish_scan, this)
        );
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int scenario_;

    sensor_msgs::msg::LaserScan create_base_scan() {
        sensor_msgs::msg::LaserScan scan;
        scan.header.frame_id = "laser_frame";
        scan.header.stamp = this->now();
        scan.angle_min = -1.57;
        scan.angle_max = 1.57;
        scan.angle_increment = 0.01;
        scan.range_min = 0.1;
        scan.range_max = 10.0;
        int size = static_cast<int>((scan.angle_max - scan.angle_min) / scan.angle_increment);
        scan.ranges.assign(size, 3.5f);
        return scan;
    }

    void set_range_block(sensor_msgs::msg::LaserScan & scan, int start, int end, float value) {
        int size = static_cast<int>(scan.ranges.size());
        start = std::max(0, start);
        end = std::min(size, end);

        for (int i = start; i < end; ++i) {
            scan.ranges[i] = value;
        }
    }

    void apply_center_obstacle(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;
        set_range_block(scan, third, 2 * third, 0.4f);
    }

    void apply_left_obstacle(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;
        set_range_block(scan, 0, third, 0.5f);
    }

    void apply_right_obstacle(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;
        set_range_block(scan, 2 * third, size, 0.5f);
    }

    void apply_fully_blocked(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        set_range_block(scan, 0, size, 0.3f);
    }

    void apply_center_left_blocked(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;
        set_range_block(scan, 0, 2 * third, 0.4f);
    }

    void apply_center_right_blocked(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;
        set_range_block(scan, third, size, 0.4f);
    }

    void apply_narrow_corridor(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;

        set_range_block(scan, 0, third, 0.8f);
        set_range_block(scan, third, 2 * third, 1.5f);
        set_range_block(scan, 2 * third, size, 0.8f);
    }

    void apply_left_tighter_than_right(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;

        set_range_block(scan, 0, third, 0.7f);
        set_range_block(scan, third, 2 * third, 1.2f);
        set_range_block(scan, 2 * third, size, 2.5f);
    }

    void apply_right_tighter_than_left(sensor_msgs::msg::LaserScan & scan) {
        int size = static_cast<int>(scan.ranges.size());
        int third = size / 3;

        set_range_block(scan, 0, third, 2.5f);
        set_range_block(scan, third, 2 * third, 1.2f);
        set_range_block(scan, 2 * third, size, 0.7f);
    }

    void publish_scan() {
        auto scan = create_base_scan();

        switch (scenario_) {
            case 0:
                RCLCPP_INFO(this->get_logger(), "Scenario: CLEAR");
                break;
            case 1:
                RCLCPP_INFO(this->get_logger(), "Scenario: CENTER BLOCKED");
                apply_center_obstacle(scan);
                break;
            case 2:
                RCLCPP_INFO(this->get_logger(), "Scenario: LEFT BLOCKED");
                apply_left_obstacle(scan);
                break;
            case 3:
                RCLCPP_INFO(this->get_logger(), "Scenario: RIGHT BLOCKED");
                apply_right_obstacle(scan);
                break;
            case 4:
                RCLCPP_INFO(this->get_logger(), "Scenario: FULLY BLOCKED");
                apply_fully_blocked(scan);
                break;
            case 5:
                RCLCPP_INFO(this->get_logger(), "Scenario: CENTER + LEFT BLOCKED");
                apply_center_left_blocked(scan);
                break;
            case 6:
                RCLCPP_INFO(this->get_logger(), "Scenario: CENTER + RIGHT BLOCKED");
                apply_center_right_blocked(scan);
                break;
            case 7:
                RCLCPP_INFO(this->get_logger(), "Scenario: NARROW CORRIDOR");
                apply_narrow_corridor(scan);
                break;
            case 8:
                RCLCPP_INFO(this->get_logger(), "Scenario: LEFT TIGHTER THAN RIGHT");
                apply_left_tighter_than_right(scan);
                break;
            case 9:
                RCLCPP_INFO(this->get_logger(), "Scenario: RIGHT TIGHTER THAN LEFT");
                apply_right_tighter_than_left(scan);
                break;
        }
        scan.header.stamp = this->now();
        publisher_->publish(scan);
        scenario_ = (scenario_ + 1) % 10;
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FakeScanNode>());
    rclcpp::shutdown();
    return 0;
}