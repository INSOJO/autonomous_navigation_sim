from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='autonomous_navigation_sim',
            executable='fake_scan_node',
            name='fake_scan_node',
            output='screen'
        ),
        Node(
            package='autonomous_navigation_sim',
            executable='perception_node',
            name='perception_node',
            output='screen'
        ),
        Node(
            package='autonomous_navigation_sim',
            executable='safety_monitor_node',
            name='safety_monitor_node',
            output='screen'
        ),
    ])