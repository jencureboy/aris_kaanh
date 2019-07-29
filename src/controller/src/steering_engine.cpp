#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>
#include "controller/rob_param.h"
#include "controller/pose.h"
#include "controller/interface.h"
#include <cmath>

#include <cstdlib>
#include "controller/setpos.h"
#include "controller/motorpos.h"

double threshold = 0.01;
bool power_on=false;
int d=0;//the directiong to move the robot 
int angle_steer = 0;//he parameter to control steering engine
int angle_get =0;

void call_service(ros::ServiceClient & cmd_client,std::string cmd_in)
{
  controller::interface srv;
  srv.request.cmd = cmd_in;
  if (cmd_client.call(srv))
  {
    ROS_INFO("Sum: %ld", (long int)srv.response.return_code);
  }
  else
  {
    //ROS_ERROR("Failed to call service controller");
    return;
  }
  return;
}

void call_service2(ros::ServiceClient & client2,uint16_t cmd_in)
{
  controller::setpos srv;
  srv.request.cmd = cmd_in;
  client2.call(srv);
  return;
}

void chatterCallBack(const controller::rob_param::ConstPtr& msg)//?????
{	
	//ROS_INFO("listener recieve from joy msg->x:[%d]", msg->x);
	ROS_INFO("in listener command box");
	if(msg->forward_back>0.00 && msg->x ==1) d=1;
    else if (msg->forward_back<0.00 && msg->x ==1) d =-1;
    else d = 0;

}
void chatterCallBack2(const controller::motorpos::ConstPtr& msg)
{
	ROS_INFO("Get angle_get: %ld", msg->angular);
	if(angle_get == 0)
	{
		angle_get = msg->angular;
	}	
	//angle_steer = angle_get;
}
int main(int argc,char ** argv)
{
	ros::init(argc, argv, "steering_engine");
	ros::NodeHandle n;
	ros::ServiceClient client2 = n.serviceClient<controller::setpos>("gettargetpos");
	//ros::Rate loop_rate(1);
	ros::Rate loop_rate(5);

	ros::Subscriber sub2 = n.subscribe("motorpos_topic",1,chatterCallBack2);

	ros::Subscriber sub = n.subscribe("robot_joy_topic",1,chatterCallBack);

	while (ros::ok())
  	{
  		angle_get += 2 * d;
		angle_get = std::max(0,angle_get);
		angle_get = std::min(350,angle_get);
  		call_service2(client2,angle_get);
    	//call_service(cmd_client,cmd);
    	ros::spinOnce();
    	//loop_rate.sleep();
  	}
	return 0;
}