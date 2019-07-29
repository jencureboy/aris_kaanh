#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>
#include <aris.hpp>
#include "kaanh.h"
#include <atomic>
#include <string>
#include <filesystem>
#include "controller/interface.h"
#include "controller/pose.h"

using namespace aris::dynamic;

auto&cs = aris::server::ControlServer::instance();

bool do_cmd(std::string command_in)
{
	try
	{
		auto target = cs.executeCmd(aris::core::Msg(command_in));
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return true;
}

bool exector(controller::interface::Request  &req,
	controller::interface::Response &res)
{
	ROS_INFO("controller: receive request from client: cmd=%s", req.cmd.c_str());
	do_cmd(req.cmd);
	res.return_code=1;
	ROS_INFO("controller: sending back response to client: [%ld]", (long int)res.return_code);
	return true;
}

int main(int argc,char ** argv)
{
	ros::init(argc, argv, "controller");
	ros::NodeHandle n;
	//controll the model in Rviz,topic: model_control
	ros::Publisher chatter_pub = n.advertise<std_msgs::String>("model_control", 1000);
	//ros::Publisher chatter_pub = n.advertise<std_msgs::String>("kannh_topic", 1000);
	ros::Rate loop_rate(10);



	ros::ServiceServer service = n.advertiseService("getcmd", exector);

	
	//auto xmlpath = std::filesystem::absolute(".");
	//const std::string xmlfile = "kaanh.xml";
	auto xmlpath = std::filesystem::absolute(".");//获取当前工程所在的路径
	auto uixmlpath = std::filesystem::absolute(".");
	const std::string xmlfile = "kaanh.xml";
	const std::string uixmlfile = "interface_kaanh.xml";
	xmlpath = xmlpath / xmlfile;
	uixmlpath = uixmlpath / uixmlfile;
	
	cs.resetController(kaanh::createControllerRokaeXB4().release());
	cs.resetModel(kaanh::createModelRokae().release());
	cs.resetPlanRoot(kaanh::createPlanRootRokaeXB4().release());
	cs.resetSensorRoot(new aris::sensor::SensorRoot);
    cs.saveXmlFile(xmlpath.string().c_str());
	
    cs.loadXmlFile(xmlpath.string().c_str());
    //cs.interfaceRoot().loadXmlFile(uixmlpath.string().c_str());
    //cs.saveXmlFile(xmlpath.string().c_str());
   	
    cs.start();

	//aris::plan::PlanTarget &target;
	//auto pq= std::vector<double>(target.model->partPool().size()*7);
	while (ros::ok())
	{
		
		auto target = cs.executeCmd(aris::core::Msg("get_part_pq"));
		target->finished.wait();
		auto &str = std::any_cast<std::string&>(target->ret);

		std::vector<double> data(49,0.0);
		std::copy(str.data(),str.data() + 49*sizeof(double), reinterpret_cast<char*>(data.data()));
		/*double pe[42]{0.0};
		aris::dynamic::s_pq2pe(data.data() + 0, pe, "123");
		aris::dynamic::s_pq2pe(data.data() + 7, pe+6, "123");
		aris::dynamic::s_pq2pe(data.data() + 14, pe+12, "123");
		aris::dynamic::s_pq2pe(data.data() + 21, pe+18, "123");
		aris::dynamic::s_pq2pe(data.data() + 28, pe+24, "123");
		aris::dynamic::s_pq2pe(data.data() + 35, pe+30, "123");
		aris::dynamic::s_pq2pe(data.data() + 42, pe+36, "123");
		
		aris::dynamic::dsp(1,7,data.data());
		aris::dynamic::dsp(1,6,pe);
		aris::dynamic::dsp(1,7,data.data() + 7);
		aris::dynamic::dsp(1,6,pe + 6);
		aris::dynamic::dsp(1,7,data.data() + 14);
		aris::dynamic::dsp(1,6,pe + 12);
		*/
		//std::cout<<"data"<<data[0]<<std::endl;
		/*aris::PlanTarget &target;
		double pq[7][7],pe[7][6];
		target.model->partPool().getpartpq(pq[0]);
		aris::dynamic::s_pq2pe(pq[0],pe[0],"313");
		*/

		//controller::pose msg;
		//controller::rob_posture msg;

		/*do_cmd(get_part_pq);
		std::cout<<"pq[0]:   "<<pq[0]<<"pq[1]:   "<<pq[1]<<std::endl;
		*/
		/*
		for(int i=0;i<42;i++)
		{
			pe[i]=0;
		}
		*/

		std_msgs::String msg;
		std::stringstream ss;
		ss<<"{";
		for(int i=0;i<49;i++)
		{
			ss<<data[i];
			if(i!=48)
			{
				ss<<",";
			}
		}
		ss<<"}";
		//ss<<"{0,0,0,0,0,0,"<<pe[0]<<","<<pe[1]<<","<<pe1[2]<<","<<pe2[3]<<","<<pe
		//ss << "moveJR -m=0 --pos=0.1";
		//ss << pq;
		//ss << "{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17.00006,18.002,19.1,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42}";
		msg.data = ss.str();
		chatter_pub.publish(msg);

		ros::spinOnce();
		loop_rate.sleep();
	}
	return 0;
}
