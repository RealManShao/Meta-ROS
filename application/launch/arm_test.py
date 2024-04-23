import os
from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, GroupAction,
                            IncludeLaunchDescription, SetEnvironmentVariable)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('application'),
        'config',
        'scara_config.yaml'
    )

    moveit_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('scara_moveit'),
                        'launch/scara.py')
        )
    )

    ld = LaunchDescription([
        Node(
            package="dji_controller",
            executable="dji_controller",
            name="dji_controller",
            parameters=[config],
        )
    ])
    
    ld.add_action(moveit_launch)

    return ld