#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>
#include "controller/rob_param.h"
#include "controller/pose.h"
#include "controller/interface.h"
#include <cmath>

//void chatterCallBack(const robot_joy::pose::ConstPtr& msg)
int vel_gear =0;//the gear initialed;
//power_on=false means joystick is defaultly disabled, mode =false means robot is defaultly on joint space
bool power_on=false, mode = false, if_en = false;
double threshold = 0.01;
//coordinate: value1 means Local coordinate system,value0 means world coordinate system
int coordinate = 0;
//attention,vmax need to be verified????????
double va_percent=0;
int d=1;//the directiong to move the robot 
std::vector<std::string> cmd_vec;
//std::string cmd;
int init=0, init1 =1;
double init_d=0.00,init_d1=1.00;
int mode_convert = 0;//the parameter is used for Conditional Judgement

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

void chatterCallBack(const controller::rob_param::ConstPtr& msg)//?????
{	
	//ROS_INFO("listener recieve from joy msg->x:[%d]", msg->x);
	ROS_INFO("in listener command box");
	//.c_str()
	cmd_vec.clear();
	if(msg->gear ==1)
	{
		vel_gear += 1;

	}
	else if(msg->gear == -1)
	{
		vel_gear -= 1;
	}
	vel_gear = std::max(0,vel_gear);
	vel_gear = std::min(5,vel_gear);
	std::cout<<"Now in the "<<vel_gear<<"th gear."<<std::endl;

	int num_b =0;
	if(msg->x ==1) num_b +=1;
	if(msg->y ==1) num_b +=1;
	if(msg->z ==1) num_b +=1;
	if(msg->rx ==1) num_b +=1;
	if(msg->ry ==1) num_b +=1;
	if(msg->rz <=-0.9) num_b +=1;
	if(msg->md_ds_button == 1) num_b +=1;
	if(abs(msg->start_stop)>threshold) num_b +=1;
	if(abs(msg->rc_en_button)>threshold) num_b +=1;
	if(msg->rs_button ==1) num_b +=1;
	if(msg->select_mode == 1) num_b +=1;
	if(msg->start == 1) num_b +=1;
	if(msg->gear == 1) num_b +=1;
	if(abs(msg->forward_back)>threshold) num_b +=1;
	if(num_b >2)
	{
		std::cout<<"listener: Please don't hold down more than two buttons at the same time."<<std::endl;
		std::cout<<"The robot is in safety mode.The gear is zeroed.Please increase gear."<<std::endl;
		vel_gear = 0;
	}
    if (msg->start == 1)
    {
    	power_on = !power_on;
    }
    //enable joystick
    if (power_on)
    {
    	 	
    	if(msg->select_mode == 1)
    	{
    		if(mode_convert == 0)
    		{
    			mode = !mode;
    		}
    		if(mode_convert == 1)
    		{
    			cmd_vec.push_back("jogJ --stop");
    			mode = !mode;
    			mode_convert = 0;
    		}
    		if(mode_convert == 2)
    		{
    			cmd_vec.push_back("jogC --stop");
    			mode = !mode;
    			mode_convert = 0;
    		}
    	}
  		if(msg->md_ds_button == 1 && 
			abs(msg->start_stop)<threshold && 
			abs(msg->rc_en_button)<threshold && 
			msg->rs_button ==0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold)
		{
		//cmd="md;en;rc";
		cmd_vec.push_back("md");
		cmd_vec.push_back("en");
		cmd_vec.push_back("rc");
		if_en = true;
		}
		if(msg->md_ds_button == -1)
		{

			if(mode_convert == 0)
    		{
    			cmd_vec.push_back("ds");
    			if_en = false;
    		}
    		if(mode_convert == 1)
    		{
    			cmd_vec.push_back("jogJ --stop");
    			mode_convert = 0;
    		}
    		if(mode_convert == 2)
    		{
    			cmd_vec.push_back("jogC --stop");
    			mode_convert = 0;
    		}
		
		}
		/*
		if(msg->rc_en_button >=0.9 && 
			abs(msg->start_stop)<threshold && 
			msg->md_ds_button ==0 && 
			msg->rs_button ==0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold)
		{
		cmd ="en";
		}
		if(msg->rc_en_button <=-0.9 && 
			abs(msg->start_stop)<threshold  && 
			msg->md_ds_button ==0 && 
			msg->rs_button ==0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold)
		{
		cmd ="rc";
		}
		*/
		
		if(msg->rs_button ==1 &&
		    abs(msg->rc_en_button)<threshold && 
			abs(msg->start_stop)<threshold && 
			msg->md_ds_button == 0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold)
		{
		cmd_vec.push_back("rs --vel=0.02");
		} 
    	if(mode)//on end space
    	{
    		
			std::cout<<"listener: now is on end space"<<std::endl;   		
    		va_percent= abs(msg->forward_back) * vel_gear *0.2*100;
    		//the value 1 means the robot is on end space
    		coordinate = 0;
    		//the value 1 means the robot is on the positive direction
    		if(msg->forward_back>0.00) d=1;
    		else d=-1;
    		if(msg->start_stop >= 0.9 && 
    		msg->rs_button ==0 && 
		    abs(msg->rc_en_button)<threshold && 	 
			msg->md_ds_button == 0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold
    			)
    		{
    			if(if_en == true)
    			{
    				cmd_vec.push_back("jogC --start");
    				mode_convert = 2;
    			}
    			else
    			{
    				std::cout<<"Please first enable the controller and then press the unlock button."<<std::endl;
    			}   			
    		}
    		else if(msg->start_stop <= -0.9)
    		{
    			if(if_en == true)
    			{
    				cmd_vec.push_back("jogC --stop");
    				mode_convert = 0;
    			}
    			else
    			{
    				std::cout<<"Please first enable the controller and then press the lock button."<<std::endl;
    			}   						
    		}
    		else if(msg->x ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->y ==0 && 
					msg->z ==0 && 
					msg->rx ==0 && 
					msg->ry ==0 && 
					msg->rz >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{
    			cmd_vec.push_back("jogC --x="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->y ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->x ==0 && 
					msg->z ==0 && 
					msg->rx ==0 && 
					msg->ry ==0 && 
					msg->rz >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    				)
    		{	
    			cmd_vec.push_back("jogC --y="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->z ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->x ==0 && 
					msg->y ==0 && 
					msg->rx ==0 && 
					msg->ry ==0 && 
					msg->rz >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    				)
    		{
    			cmd_vec.push_back("jogC --z="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->rx ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->x ==0 && 
					msg->y ==0 && 
					msg->z ==0 && 
					msg->ry ==0 && 
					msg->rz >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{    			
    			cmd_vec.push_back("jogC --a="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->ry ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->x ==0 && 
					msg->y ==0 && 
					msg->z ==0 && 
					msg->rx ==0 && 
					msg->rz >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{ 			
    			cmd_vec.push_back("jogC --b="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->rz <= -0.9 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->x ==0 && 
					msg->y ==0 && 
					msg->z ==0 && 
					msg->rx ==0 && 
					msg->ry >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    		)//the default value of msg.rz & msg.j6 is 1
    		{ 			
    			cmd_vec.push_back("jogC --c="+std::to_string(d) +" --cor=" + std::to_string(coordinate) +" --vel_percent=" + std::to_string(va_percent));
    		}
    		else
    		{
    			std::cout<<"listener: You can manipulate the direction key to control the robot.";
    		}
    	}

    	


    	if(!mode)//on joint space
    	{
   			std::cout<<"listener: now is on joint space"<<std::endl;   		
    		va_percent= abs(msg->forward_back) * vel_gear *0.2*100;
    		if(msg->forward_back>0) d=1;
    		else d=-1;
    		if(
    		msg->start_stop >= 0.9 && 
    		msg->rs_button ==0 && 
		    abs(msg->rc_en_button)<threshold && 	 
			msg->md_ds_button == 0 && 
			msg->x ==0 && 
			msg->y ==0 && 
			msg->z ==0 && 
			msg->rx ==0 && 
			msg->ry ==0 && 
			msg->rz >=-0.9 && 
			msg->select_mode == 0 && 
			msg->start == 0 && 
			msg->gear == 0 && 
			abs(msg->forward_back)<threshold
    			)
    		{
    			if(if_en == true)
    			{
    				cmd_vec.push_back("jogJ --start");
    				mode_convert = 1;
    			}
    			else
    			{
    				std::cout<<"Please first enable the controller and then press the unlock button."<<std::endl;
    			}     			
    		}
    		else if(msg->start_stop <= -0.9)
    		{
    			if(if_en == true)
    			{
    				cmd_vec.push_back("jogJ --stop");
    				mode_convert = 0;
    			}
    			else
    			{
    				std::cout<<"Please first enable the controller and then press the unlock button."<<std::endl;
    			}     				
    		}
    		else if(msg->j1==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j2 ==0 && 
					msg->j3 ==0 && 
					msg->j4 ==0 && 
					msg->j5 ==0 && 
					msg->j6 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{
    			cmd_vec.push_back("jogJ -m=0 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->j2==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j1 ==0 && 
					msg->j3 ==0 && 
					msg->j4 ==0 && 
					msg->j5 ==0 && 
					msg->j6 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{
    			cmd_vec.push_back("jogJ -m=1 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->j3 ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j1 ==0 && 
					msg->j2 ==0 && 
					msg->j4 ==0 && 
					msg->j5 ==0 && 
					msg->j6 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{
    			cmd_vec.push_back("jogJ -m=2 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->j4 ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j1 ==0 && 
					msg->j2 ==0 && 
					msg->j3 ==0 && 
					msg->j5 ==0 && 
					msg->j6 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{	
    			cmd_vec.push_back("jogJ -m=3 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->j5 ==1 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j1 ==0 && 
					msg->j2 ==0 && 
					msg->j3 ==0 && 
					msg->j4 ==0 && 
					msg->j6 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{
    			cmd_vec.push_back("jogJ -m=4 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else if(msg->j6 <=-0.9 && abs(msg->forward_back) > threshold && 
    				abs(msg->start_stop)<threshold && 
    				msg->rs_button ==0 && 
		    		abs(msg->rc_en_button)<threshold && 	 
					msg->md_ds_button == 0 && 
					msg->j1 ==0 && 
					msg->j2 ==0 && 
					msg->j3 ==0 && 
					msg->j4 ==0 && 
					msg->j5 >=-0.9 && 
					msg->select_mode == 0 && 
					msg->start == 0 && 
					msg->gear == 0
    			)
    		{	
    			cmd_vec.push_back("jogJ -m=5 --direction=" + std::to_string(d)+" --vel_percent=" + std::to_string(va_percent));
    		}
    		else
    		{
    			std::cout<<"listener: You can manipulate the direction key to control the robot.";
    		}
    	}
    	if( abs(msg->start_stop)<threshold &&
    		abs(msg->rc_en_button)<threshold && 
    		msg->md_ds_button == 0 && 
    		msg->rs_button ==0 && 
    		msg->x ==0 && 
    		msg->y ==0 && 
    		msg->z ==0 && 
    		msg->rx ==0 && 
    		msg->ry ==0 && 
    		msg->rz >=-0.9 && 
    		msg->select_mode == 0 && 
    		msg->start == 0 && 
    		msg->gear == 0 && 
    		abs(msg->forward_back)<threshold)
    	{
    		//cmd = "";
    		cmd_vec.push_back("");
    	}
    }
    else
    {
    	std::cout<<"listener: joystick is disabled, press start button to enable";
    }
}
int main(int argc,char ** argv)
{
	ros::init(argc, argv, "listener");
	ros::NodeHandle n;
	ros::ServiceClient cmd_client = n.serviceClient<controller::interface>("getcmd");
	ros::Rate loop_rate(1);
	ros::Subscriber sub = n.subscribe("robot_joy_topic",1,chatterCallBack);
	while (ros::ok())
  	{
  		for(int i=0; i<cmd_vec.size(); i++)
  		{
  			call_service(cmd_client,cmd_vec[i]);
  		}
    	//call_service(cmd_client,cmd);
    	ros::spinOnce();
    	//loop_rate.sleep();
  	}
	return 0;
}