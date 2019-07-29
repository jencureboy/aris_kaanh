#include<iostream>
#include <string>
#include "std_msgs/String.h"
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <robot_state_publisher/robot_state_publisher.h>
#include <tf/transform_broadcaster.h>
using namespace std;

stringstream ss;
double data[49];

void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
   ROS_INFO("I heard: [%s]", msg->data.c_str());
   stringstream ss(msg->data);
   char c;
   //the content of c is "{";
   ss>>c;
   //double data[42];
   for(int i=0;i<49;++i)
   {
    ss>>data[i];
    //the content of c is Symbol",";
    ss>>c;
    }
    std::cout<<"data[17]"<<data[17]<<std::endl;
    std::cout<<"data[18]"<<data[18]<<std::endl;
    std::cout<<"data[19]"<<data[19]<<std::endl;
    std::cout<<"data[20]"<<data[20]<<std::endl;
    
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "move_Node"); //节点的名称
    ros::NodeHandle n;
    //ros::Publisher joint_pub = n.advertise<sensor_msgs::JointState>("joint_states", 1); 

    for(int i=1;i<8;i++)
{
    data[7*i-1]=1.0;
}

    tf::TransformBroadcaster broadcaster;   
    //设置一个发布者，将消息topic(joint_states)发布出去,发布到相应的节点中去

    /*const double degree = M_PI/180;
    const double radius = 2;
    int i=-69;
    // robot state
    double angle= 0;*/;
    // message declarations
    geometry_msgs::TransformStamped odom_trans;
    odom_trans.header.frame_id = "odom";  
    odom_trans.child_frame_id = "base_link";
    geometry_msgs::TransformStamped odom_trans2;
    odom_trans2.header.frame_id = "odom";  
    odom_trans2.child_frame_id = "Link_1";
    geometry_msgs::TransformStamped odom_trans3;
    odom_trans3.header.frame_id = "odom";  
    odom_trans3.child_frame_id = "Link_2";
    geometry_msgs::TransformStamped odom_trans4;
    odom_trans4.header.frame_id = "odom";  
    odom_trans4.child_frame_id = "Link_3";
    geometry_msgs::TransformStamped odom_trans5;
    odom_trans5.header.frame_id = "odom";  
    odom_trans5.child_frame_id = "Link_4";
    geometry_msgs::TransformStamped odom_trans6;
    odom_trans6.header.frame_id = "odom";  
    odom_trans6.child_frame_id = "Link_5";
    geometry_msgs::TransformStamped odom_trans7;
    odom_trans7.header.frame_id = "odom";  
    odom_trans7.child_frame_id = "Link_6";   

    
    

    //subscribe message from controller
    ros::Subscriber sub = n.subscribe("model_control",1,chatterCallback);

    std::cout<<"data[0]"<<data[0]<<std::endl;
    std::cout<<"data[16]"<<data[16]<<std::endl;
    std::cout<<"data[17]"<<data[17]<<std::endl;
    std::cout<<"data[18]"<<data[18]<<std::endl;
    ros::Rate loop_rate(10);    //这个设置的太大，效果很不好，目前觉得为10最好了    
    //sensor_msgs::JointState joint_state;
    //  for(int j=0;j<6;++j)
    // {
        //joint_state.velocity.push_back(1);
        //joint_state.effort.push_back(200);
    //}



    while (ros::ok()) 
    {
        //update joint_state
        /*joint_state.header.stamp = ros::Time::now();
        joint_state.name.resize(1);
        joint_state.position.resize(1);
        joint_state.name[0]="Joint_1";
        joint_state.position[0] = i*degree;*/



        // update transform  
        // (moving in a circle with radius)  

        odom_trans.header.stamp = ros::Time::now();  
     //odom_trans.transform.translation.x = convert<int>(x); 
        odom_trans.transform.translation.x = data[0];  
        odom_trans.transform.translation.y = data[1];  
        odom_trans.transform.translation.z = data[2];  
        odom_trans.transform.rotation.x = data[3]; 
        odom_trans.transform.rotation.y = data[4]; 
        odom_trans.transform.rotation.z = data[5]; 
        odom_trans.transform.rotation.w = data[6]; 
        //odom_trans.transform.rotation = tf::createQuaternionMsgFromRollPitchYaw(data[3], data[4],data[5]);

        /*std::cout<<odom_trans.transform.rotation.x<<"4yuanshudadaxiao"<<std::endl;
        std::cout<<odom_trans.transform.rotation.y<<"y"<<std::endl;
        std::cout<<odom_trans.transform.rotation.z<<"z"<<std::endl;
        std::cout<<odom_trans.transform.rotation.w<<"w"<<std::endl;
        std::cout<<odom_trans.transform.rotation<<"4yuanshudadaxiao"<<std::endl;
        */

        odom_trans2.header.stamp = ros::Time::now();  
        odom_trans2.transform.translation.x = data[7];  
        odom_trans2.transform.translation.y = data[8];  
        odom_trans2.transform.translation.z = data[9];  
        odom_trans2.transform.rotation.x = data[10]; 
        odom_trans2.transform.rotation.y = data[11]; 
        odom_trans2.transform.rotation.z = data[12]; 
        odom_trans2.transform.rotation.w = data[13]; 

        odom_trans3.header.stamp = ros::Time::now();  
        odom_trans3.transform.translation.x = data[14];  
        odom_trans3.transform.translation.y = data[15];  
        odom_trans3.transform.translation.z = data[16];  
        odom_trans3.transform.rotation.x = data[17]; 
        odom_trans3.transform.rotation.y = data[18]; 
        odom_trans3.transform.rotation.z = data[19]; 
        odom_trans3.transform.rotation.w = data[20]; 

        odom_trans4.header.stamp = ros::Time::now();  
        odom_trans4.transform.translation.x = data[21];  
        odom_trans4.transform.translation.y = data[22];  
        odom_trans4.transform.translation.z = data[23];  
        odom_trans4.transform.rotation.x = data[24]; 
        odom_trans4.transform.rotation.y = data[25]; 
        odom_trans4.transform.rotation.z = data[26]; 
        odom_trans4.transform.rotation.w = data[27]; 

        odom_trans5.header.stamp = ros::Time::now();  
        odom_trans5.transform.translation.x = data[28];  
        odom_trans5.transform.translation.y = data[29];  
        odom_trans5.transform.translation.z = data[30];  
        odom_trans5.transform.rotation.x = data[31]; 
        odom_trans5.transform.rotation.y = data[32]; 
        odom_trans5.transform.rotation.z = data[33]; 
        odom_trans5.transform.rotation.w = data[34]; 

        odom_trans6.header.stamp = ros::Time::now();  
        odom_trans6.transform.translation.x = data[35];  
        odom_trans6.transform.translation.y = data[36];  
        odom_trans6.transform.translation.z = data[37];  
        odom_trans6.transform.rotation.x = data[38]; 
        odom_trans6.transform.rotation.y = data[39]; 
        odom_trans6.transform.rotation.z = data[40]; 
        odom_trans6.transform.rotation.w = data[41]; 

        odom_trans7.header.stamp = ros::Time::now();  
        odom_trans7.transform.translation.x = data[42];  
        odom_trans7.transform.translation.y = data[43];  
        odom_trans7.transform.translation.z = data[44];  
        odom_trans7.transform.rotation.x = data[45]; 
        odom_trans7.transform.rotation.y = data[46]; 
        odom_trans7.transform.rotation.z = data[47]; 
        odom_trans7.transform.rotation.w = data[48]; 


        /*if(i<=70)
            i++;
        else
            i=-69; 
        */
        //send the joint state and transform
        //joint_pub.publish(joint_state);
        /*std::cout<<"data[0]"<<data[0]<<std::endl;
        std::cout<<"data[16]"<<data[16]<<std::endl;
        std::cout<<"data[17]"<<data[17]<<std::endl;
        std::cout<<"data[18]"<<data[18]<<std::endl;
        std::cout<<"data[18]"<<data[18]<<std::endl;
        std::cout<<"odom_trans4.transform.translation.x"<<odom_trans4.transform.translation.x<<std::endl;*/
        broadcaster.sendTransform(odom_trans);
        broadcaster.sendTransform(odom_trans2);
        broadcaster.sendTransform(odom_trans3);
        broadcaster.sendTransform(odom_trans4);
        broadcaster.sendTransform(odom_trans5);
        broadcaster.sendTransform(odom_trans6);
        broadcaster.sendTransform(odom_trans7);
        ros::spinOnce();
        // Create new car angle
        //angle += degree/4;

        // This will adjust as needed per iteration
        //loop_rate.sleep();
   }

   return 0;
}

