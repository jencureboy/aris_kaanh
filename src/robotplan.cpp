﻿#include <math.h>
#include"kaanh.h"
#include <algorithm>
#include"robotplan.h"
#include"planfuns.h"
#include <vector>
//using namespace std;
using namespace aris::plan;
using namespace aris::dynamic;
using namespace PLANGK;
/// \brief

planconfig planoperator;


void RealU(const int count, const int count_change, const double u0, const double u1, double &u)
{
	double tau = (count - count_change) / 10.0;
	if (tau > 1)
		tau = 1;

	u = u0 + (u1 - u0)*(3 * tau*tau - 2 * tau*tau*tau);

}


struct DoubleSPlanParam
{
	double jMax, aMax, vMax, sMax;
	double T[4];
	double alim, vlim;

};
auto DoubleSPlan::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	DoubleSPlanParam param;

	for (auto &p : params)
	{
		if (p.first == "jMax")
			param.jMax = std::stod(p.second);
		if (p.first == "aMax")
			param.aMax = std::stod(p.second);
		if (p.first == "vMax")
			param.vMax = std::stod(p.second);
		if (p.first == "sMax")
			param.sMax = std::stod(p.second);

	}
	double begin_pjs[6];
	for (int i=0;i<6;i++)
		begin_pjs[i] = target.model->motionPool()[i].mp();



	double vlim=0, alim=0;
	double T[4] = { 0 };//T[0]:Tj;  T[1]:Ta;  T[2]:Tv;  T[3]:Tmin
	planoperator.DoubleSVelocity(param.jMax, param.aMax, param.vMax, param.sMax, T,vlim,alim);

	double Tnorm = round(T[3] * 1000 + 1) / 1000.0;
	double k = T[3] / Tnorm;
	param.vMax = param.vMax*k;
	param.aMax = param.aMax*k*k;
	param.jMax = param.jMax*k*k*k;
	planoperator.DoubleSVelocity(param.jMax, param.aMax, param.vMax, param.sMax, T,vlim,alim);


	param.vlim = vlim;
	param.alim = alim;
	std::copy(T,T+4,param.T);

	target.param = param;

	for (auto &option : target.mot_options) option |=
		Plan::USE_TARGET_POS |
		//#ifdef WIN32
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
		Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
		//#endif
		Plan::NOT_CHECK_VEL_CONTINUOUS |
		Plan::NOT_CHECK_ENABLE;
}
auto DoubleSPlan::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<DoubleSPlanParam&>(target.param);

	static double begin_pjs[6];
	static double step_pjs[6];
	static int CollectNum = 1;

	// 访问主站 //
	auto controller = target.controller;

	// 打印电流 //
	auto &cout = controller->mout();

	// 获取当前起始点位置 //
	if (target.count == 1)
		for (int i = 0; i < 6; i++)
		{
			begin_pjs[i] = target.model->motionPool()[i].mp();
			step_pjs[i] = target.model->motionPool()[i].mp();
		}
	



	if (target.model->solverPool().at(1).kinPos())return -1;

	double st = 0, vt = 0, at = 0, jt = 0, ut = 1;
	static double ut0 = ut, ut1 = ut;
	static int count_change=0;
	static double time_SVEL = 0;

	if (target.count == 400)
	{
		count_change = target.count;
		ut0 = ut1;ut1 = 0.0;
	}
	if (target.count == 500)
	{
		count_change = target.count;
		ut0 = ut1;ut1 = 2.0;
	}

	RealU(target.count, count_change, ut0, ut1, ut);
	time_SVEL = time_SVEL + ut * 0.001;


	static int finish_count = round(param.T[3] * 1000.0);
	if(time_SVEL<= param.T[3])
		planoperator.RealSVelocity(time_SVEL, param.jMax, param.aMax, param.vMax, param.sMax, param.T, param.vlim, param.alim, st, vt, at, jt);

	double tarJoint = begin_pjs[0] + param.sMax;
	int signMotion;
	if(tarJoint - begin_pjs[0]>=0)
		signMotion = 1;
	else
		signMotion = -1;

	if (time_SVEL <= param.T[3])
	{
		step_pjs[0] = begin_pjs[0] - signMotion * st;
		target.model->motionPool().at(0).setMp(step_pjs[0]);
	}




	if (target.count % 10 == 0)
	{
		//for (int i = 0; i < 6; i++)
		{
			
			//cout << "vel" << i + 1 << ":" << target.model->motionPool()[i].mv() << "  ";
			//cout << "cur" << i + 1 << ":" << target.model->motionPool()[i].ma() << "  ";
		}
		cout << step_pjs[0] << "  ";
		cout << std::endl;
	}

	auto &lout = controller->lout();
	lout << st << ",";lout << vt << ",";
	lout << at << ",";lout << jt << ",";
	lout << std::endl;
	
	if (time_SVEL >= param.T[3])
		return 0;

	return 5000 - target.count;
}


auto DoubleSPlan::collectNrt(aris::plan::PlanTarget &target)->void
{

}


DoubleSPlan::DoubleSPlan(const std::string &name) :Plan(name)
{

	command().loadXmlStr(
		"<Command name=\"SPlan\">"
		"	<GroupParam>"
		"		<Param name=\"jMax\"default=\"120.0\"/>"
		"		<Param name=\"aMax\"default=\"15.0\"/>"
		"		<Param name=\"vMax\" default=\"4.0\"/>"
		"		<Param name=\"sMax\"default=\"1.5\"/>"
		"	</GroupParam>"
		"</Command>");

}






