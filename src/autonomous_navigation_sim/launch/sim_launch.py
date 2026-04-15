from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_share = get_package_share_directory('autonomous_navigation_sim')
    rviz_config = os.path.join(pkg_share, 'rviz', 'autonomous_navigation_sim.rviz')

    return LaunchDescription([
        Node(
            package='autonomous_navigation_sim',
            executable='fake_scan_node',
            name='fake_scan_node'
        ),
        Node(
            package='autonomous_navigation_sim',
            executable='perception_node',
            name='perception_node'
        ),
        Node(
            package='autonomous_navigation_sim',
            executable='decision_node',
            name='decision_node'
        ),
        Node(
            package='autonomous_navigation_sim',
            executable='safety_monitor_node',
            name='safety_monitor_node'
        ),
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='laser_tf_publisher',
            arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'laser_frame']
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        ),
    ])