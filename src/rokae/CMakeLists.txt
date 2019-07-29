cmake_minimum_required(VERSION 2.8.3)

project(rokae)


#set(CMAKE_CXX_STANDARD 17)
add_compile_options(-std=c++11)

find_package(catkin REQUIRED  COMPONENTS roscpp rospy std_msgs genmsg tf
	moveit_core
	moveit_visual_tools
	moveit_ros_planning
	moveit_ros_planning_interface)

catkin_package(CATKIN_DEPENDS roscpp rospy std_msgs genmsg tf
	moveit_core
	moveit_visual_tools
	moveit_ros_planning
	moveit_ros_planning_interface)

include_directories(include ${catkin_INCLUDE_DIRS})

#add_executable(move_rokae src/move.cpp)
add_executable(move_Node2 src/move.cpp)
target_link_libraries(move_Node2 ${catkin_LIBRARIES})

find_package(roslaunch)

#my be wrong add_library(moveit ${catkin_LIBRARIES})


foreach(dir config launch meshes urdf)
	install(DIRECTORY ${dir}/
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach(dir)