﻿#include "sixdistalfc.h"
#include <math.h>
#include"robotconfig.h"
#include"sixdistaldynamics.h"
#include <vector>
using namespace std;
using namespace aris::plan;
using namespace aris::dynamic;
using namespace CONFIG;
using namespace sixDistalDynamicsInt;
/// \brief





robotconfig robotDemo;
sixdistaldynamics sixDistalMatrix;
double estParas[GroupDim]={-0.03543402,0.04347624,0.05651912,-0.04722695,-0.05991794,-0.08057128,0.02379530,-0.00348242,0.02249035,
                           -0.39696606,-0.21818207,-0.21345102,1.70374386,-3.39132646,-10.60129932,-0.26885910,-0.13424096,-1.70564392,
                           -0.69093494,-4.31449348,-0.16970778,0.29115017,1.74100845,2.26599670,4.22193900,-0.01429569,-0.05549581,
                           -0.00492963,-0.07884729,-0.06306367,0.01734506,-0.03780796,-0.01712891,-1.14768852,-0.46078009,0.00188965,
                           -0.00727051,0.03497930,-0.36029486,-1.08830666,-24.11968458,-27.17416365,-16.19901088,-1.92030904,-0.64581102,1.14128814};


std::vector<double> PositionList_vec(6 * SampleNum);
auto PositionList = PositionList_vec.data();
std::vector<double> SensorList_vec(6 * SampleNum);
auto SensorList = SensorList_vec.data();

double StatisError[6];

struct MoveXYZParam
{
	double damp[6];

};
auto MoveXYZ::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
    MoveXYZParam param;


	for (auto &p : params)
	{

		if (p.first == "damp")
		{
			for (int i = 0; i < 6; i++)
			   param.damp[i] = std::stod(p.second);
		}
	
	}
	target.param = param;

	target.option |=
		Plan::USE_TARGET_POS |
#ifdef WIN32
		Plan::NOT_CHECK_POS_MIN |
		Plan::NOT_CHECK_POS_MAX |
		Plan::NOT_CHECK_POS_CONTINUOUS |
		Plan::NOT_CHECK_POS_CONTINUOUS_AT_START |
        Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
		Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
#endif
            Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
            Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
		Plan::NOT_CHECK_VEL_MIN |
		Plan::NOT_CHECK_VEL_MAX |
		Plan::NOT_CHECK_VEL_CONTINUOUS |
		Plan::NOT_CHECK_VEL_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_VEL_FOLLOWING_ERROR;
	
   
}


void crossVector(const double* a, const double* b, double* c)
{
	
	c[0] = a[1] * b[2] - b[1] * a[2];
	c[1] = -(a[0] * b[2] - b[0] * a[2]);
	c[2] = a[0] * b[1] - b[0] * a[1];

}

auto MoveXYZ::executeRT(PlanTarget &target)->int
{
			auto &param = std::any_cast<MoveXYZParam&>(target.param);
			
			double RobotPosition[6];
			double RobotPositionJ[6];
			double RobotVelocity[6];
			double RobotAcceleration[6];
			double TorqueSensor[6];
			double X1[3];
			double X2[3];
			static double begin_pjs[6];
			static double step_pjs[6];
            static double stateTor0[6][3],stateTor1[6][3];
            static float FT0[6];

            // 访问主站 //
            auto controller = target.controller;

				// 获取当前起始点位置 //
				if (target.count == 1)
				{
                    for (int i = 0; i < 6; ++i)
					{
						step_pjs[i] = target.model->motionPool()[i].mp();
                       // controller->motionPool().at(i).setModeOfOperation(10);	//切换到电流控制
                    }
				}
				

			if (!target.model->solverPool().at(1).kinPos())return -1;


			///* Using Jacobian, TransMatrix from ARIS
			double EndW[3], EndP[3], BaseV[3];
			double PqEnd[7], TransVector[16];
			target.model->generalMotionPool().at(0).getMpm(TransVector);
            target.model->generalMotionPool().at(0).getMpq(PqEnd);

            double dX[6] = { 0.00001, -0.0000, -0.0000, -0.0000, -0.0000, -0.0000};
            double dTheta[6]={0};

            float FT[6];
            uint16_t FTnum;
            auto conSensor = dynamic_cast<aris::control::EthercatController*>(target.controller);
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x00, &FTnum ,16);
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x01, &FT[0] ,32);  //Fx
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x02, &FT[1], 32);  //Fy
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x03, &FT[2], 32);  //Fz
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x04, &FT[3], 32);
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x05, &FT[4], 32);
            conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x06, &FT[5], 32);

            FT[0]=-FT[0];FT[3]=-FT[3];



            for (int i = 0;i < 6;i++)
            {
                RobotPositionJ[i] = target.model->motionPool()[i].mp();
                RobotPosition[i] = target.model->motionPool()[i].mp();
                RobotVelocity[i]=0;
                RobotAcceleration[i]=0;
                TorqueSensor[i]=FT[i];
            }
            double estFT[6]={0};

            sixDistalMatrix.sixDistalCollision(RobotPosition, RobotVelocity, RobotAcceleration, TorqueSensor, estParas, estFT);


            for (int i = 0; i < 6; i++)
            {
                    FT[i]=FT[i]-estFT[i];
            }



            for (int i = 0; i < 3; i++)
            {
                if (abs(FT[i]) < 2)
                      FT[i]=0;
            }
            for (int i = 3; i < 6; i++)
            {
                if (abs(FT[i]) < 0.05)
                      FT[i]=0;
            }


            double temp;
            temp=FT[0];FT[0]=FT[2];FT[2]=temp;
            temp=FT[3];FT[3]=FT[5];FT[5]=temp;
            FT[1]=-FT[1];FT[5]=-FT[5];


			double FmInWorld[6];

			double TransMatrix[4][4];
			for (int i = 0;i < 4;i++)
				for (int j = 0;j < 4;j++)
					TransMatrix[i][j] = TransVector[4 * i + j];

			double n[3] = { TransMatrix[0][0], TransMatrix[0][1], TransMatrix[0][2] };
			double o[3] = { TransMatrix[1][0], TransMatrix[1][1], TransMatrix[1][2] };
			double a[3] = { TransMatrix[2][0], TransMatrix[2][1], TransMatrix[2][2] };
			
            //FT[0] = 0;FT[1] = 0;FT[2] = 1;FT[3] = 0;FT[4] = 0;FT[5] = 0;
			FmInWorld[0] = n[0] * FT[0] + n[1] * FT[1] + n[2] * FT[2];
			FmInWorld[1] = o[0] * FT[0] + o[1] * FT[1] + o[2] * FT[2];
			FmInWorld[2] = a[0] * FT[0] + a[1] * FT[1] + a[2] * FT[2];
			FmInWorld[3] = n[0] * FT[3] + n[1] * FT[4] + n[2] * FT[5];
			FmInWorld[4] = o[0] * FT[3] + o[1] * FT[4] + o[2] * FT[5];
			FmInWorld[5] = a[0] * FT[3] + a[1] * FT[4] + a[2] * FT[5];

            //robotDemo.forceTransform(RobotPositionJ, FT, FmInWorld);


            // 获取当前起始点位置 //
            if (target.count == 1)
            {

                for (int j = 0; j < 6; j++)
                    stateTor0[j][0] = FmInWorld[j];
            }


            for (int j = 0; j < 6; j++)
                {
					double A[3][3],B[3],CutFreq=10;
					A[0][0] = 0; A[0][1] = 1; A[0][2] = 0;
					A[1][0] = 0; A[1][1] = 0; A[1][2] = 1;
					A[2][0] = -CutFreq * CutFreq * CutFreq;
					A[2][1] = -2 * CutFreq * CutFreq;
					A[2][2] = -2 * CutFreq;
					B[0] = 0; B[1] = 0;
					B[2] = -A[2][0];
					double intDT = 0.001;
                    stateTor1[j][0] = stateTor0[j][0] + intDT * (A[0][0] * stateTor0[j][0] + A[0][1] * stateTor0[j][1] + A[0][2] * stateTor0[j][2] + B[0] * FmInWorld[j]);
                    stateTor1[j][1] = stateTor0[j][1] + intDT * (A[1][0] * stateTor0[j][0] + A[1][1] * stateTor0[j][1] + A[1][2] * stateTor0[j][2] + B[1] * FmInWorld[j]);
                    stateTor1[j][2] = stateTor0[j][2] + intDT * (A[2][0] * stateTor0[j][0] + A[2][1] * stateTor0[j][1] + A[2][2] * stateTor0[j][2] + B[2] * FmInWorld[j]);
                }
			   
               dX[0] = 1*stateTor1[0][0] / 80000;
               dX[1] = 1*stateTor1[1][0] / 80000;
               dX[2] = 1*stateTor1[2][0] / 80000;
               dX[3] = 1*stateTor1[3][0] / 12000;
               dX[4] = 1*stateTor1[4][0] / 12000;
               dX[5] = 1*stateTor1[5][0] / 12000;



               if (target.count % 100 == 0)
               {
                   for (int i = 0; i < 6; i++)
                   {
                       cout<<stateTor1[i][0]<<"***"<<FT[i]<<"***"<<FmInWorld[i]<<endl;

                   }

                   cout << std::endl;

               }


			   for (int j = 0; j < 6; j++)
			   {
                   if (dX[j] > 0.00025)
                       dX[j] = 0.00025;
                   if (dX[j] < -0.00025)
                       dX[j] = -0.00025;
			   }


               // 打印电流 //
               auto &cout = controller->mout();





               // log 电流 //
               auto &lout = controller->lout();

              // lout << target.model->motionPool()[0].mp() << ",";
               //lout << target.model->motionPool()[1].mp() << ",";
               //lout << target.model->motionPool()[2].mp() << ",";
               //lout << target.model->motionPool()[3].mp() << ",";
               //lout << target.model->motionPool()[4].mp() << ",";
               //lout << target.model->motionPool()[5].mp() << ",";


               lout << FT[1] << ",";lout << FT[2] << ",";
               lout << FT[3] << ",";lout << FT[4] << ",";
               lout << FT[5] << ",";lout << FT[6] << ",";
               lout << stateTor0[0][0] << ",";lout << stateTor0[1][0] << ",";
               lout << stateTor0[2][0] << ",";lout << stateTor0[3][0] << ",";
               lout << stateTor0[4][0] << ",";lout << stateTor0[5][0] << ",";

               //lout << stateTor1[2][0] << ",";lout << FT0[3] << ",";
              // lout << dX[0] << ",";

              // lout << dX[0] << ",";
              // lout << FT[1] << ",";lout << FT[2] << ",";
              // lout << FT[3] << ",";lout << FT[4] << ",";
              // lout << FT[5] << ",";lout << FT[6] << ",";
               lout << std::endl; 



               ///* Using Jacobian, TransMatrix from ARIS
		       for (int i = 0;i < 3;i++)
				   EndW[i] = dX[i+3];

			   for (int i = 0;i < 3;i++)
				   EndP[i] = PqEnd[i];
			   crossVector(EndP, EndW, BaseV);

			   for (int i = 0;i < 3;i++)
				   dX[i + 3] = dX[i + 3];
			   for (int i = 0;i < 3;i++)
				   dX[i] = dX[i]+ BaseV[i];


				auto &fwd = dynamic_cast<aris::dynamic::ForwardKinematicSolver&>(target.model->solverPool()[1]);
				fwd.cptJacobi();
				double pinv[36];

				// 所需的中间变量，请对U的对角线元素做处理
				double U[36], tau[6];
				aris::Size p[6];
				aris::Size rank;

				// 根据 A 求出中间变量，相当于做 QR 分解 //
				// 请对 U 的对角线元素做处理
				s_householder_utp(6, 6, fwd.Jf(), U, tau, p, rank, 1e-10);
				for (int i = 0;i < 6;i++)
					if (U[7 * i] >= 0)
						U[7 * i] = U[7 * i] + 0.1;
					else
						U[7 * i] = U[7 * i] - 0.1;
				// 根据QR分解的结果求x，相当于Matlab中的 x = A\b //
				s_householder_utp_sov(6, 6, 1, rank, U, tau, p, dX, dTheta, 1e-10);

				// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A) //
				double tau2[6];
				s_householder_utp2pinv(6, 6, rank, U, tau, p, pinv, tau2, 1e-10);

				// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A)*b //
				s_mm(6, 1, 6, pinv, dX, dTheta);



                for (int i = 0; i < 6; i++)
                {
                    if (dTheta[i] > 0.003)
                        dTheta[i] = 0.003;
                    if (dTheta[i] < -0.003)
                       dTheta[i] = -0.003;
                     //lout << dTheta[i] << ",";
                }


                //lout << std::endl;

				for (int i = 0; i < 6; i++)
				{
					step_pjs[i] = step_pjs[i] + dTheta[i];
                    target.model->motionPool().at(i).setMp(step_pjs[i]);
				}

                for (int i = 0; i < 6; i++)
                {
                    dTheta[i] = dTheta[i] * DirectionFlag[i];
 
                }
                     
               
                for (int i = 0; i < 6; i++)
                {

                    stateTor0[i][0] = stateTor1[i][0];
                    stateTor0[i][1] = stateTor1[i][1];
                    stateTor0[i][2] = stateTor1[i][2];
                }

				/*
                for (int i = 0; i < 6; i++)
                {
                    stateDm0[i, 0] = stateDm1[i, 0];
                    stateDm0[i, 1] = stateDm1[i, 1];
                    stateDm0[i, 2] = stateDm1[i, 2];

                    stateTor0[i, 0] = stateTor1[i, 0];
                    stateTor0[i, 1] = stateTor1[i, 1];
                    stateTor0[i, 2] = stateTor1[i, 2];
                } */
            
                return 150000000 - target.count;
}

MoveXYZ::MoveXYZ(const std::string &name) :Plan(name)
    {
        command().loadXmlStr(
            "<Command name=\"mvXYZ\">"
			"	<GroupParam>"
			"		<Param name=\"damp\" default=\"1.0\"/>"
			"	</GroupParam>"
            "</Command>");


    }


struct MoveDistalParam
{
	double period;
	double amplitude;
	
};
auto MoveDistal::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	MoveDistalParam param;

	for (auto &p : params)
	{
		if (p.first == "period")
		    param.period = std::stod(p.second);
		if (p.first == "amplitude")
			param.amplitude = std::stod(p.second);

	}

	target.param = param;

	target.option |=
		Plan::USE_TARGET_POS |
//#ifdef WIN32
		Plan::NOT_CHECK_POS_MIN |
		Plan::NOT_CHECK_POS_MAX |
		Plan::NOT_CHECK_POS_CONTINUOUS |
		Plan::NOT_CHECK_POS_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
		Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
//#endif
		Plan::NOT_CHECK_VEL_MIN |
		Plan::NOT_CHECK_VEL_MAX |
		Plan::NOT_CHECK_VEL_CONTINUOUS |
		Plan::NOT_CHECK_VEL_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_VEL_FOLLOWING_ERROR;


}
auto MoveDistal::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<MoveDistalParam&>(target.param);

	static double begin_pjs[6];
	static double step_pjs[6];
	static double perVar = 0;
	static double ampVar = 0;

	if (target.count < 1000)
	{
		ampVar = ampVar + param.amplitude / 1000;
	}
		// 获取当前起始点位置 //
	if (target.count == 1)
	{
			for (int i = 0; i < 6; i++)
			{
				begin_pjs[i] = target.model->motionPool()[i].mp();
				step_pjs[i] = target.model->motionPool()[i].mp();
			}
	}

            step_pjs[4] = begin_pjs[4] + 2*ampVar * (std::sin(2 * aris::PI / param.period *target.count/1000));
           // target.model->motionPool().at(4).setMp(step_pjs[4]);

            step_pjs[5] = begin_pjs[5] + 2*ampVar * (std::sin(2 * aris::PI / param.period *target.count/1000));
           // target.model->motionPool().at(5).setMp(step_pjs[5]);
	
    if (!target.model->solverPool().at(1).kinPos())return -1;

	// 访问主站 //
	auto controller = target.controller;

    float FT[6];
    int16_t FTnum;
    auto conSensor = dynamic_cast<aris::control::EthercatController*>(target.controller);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x00, &FTnum ,16);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x01, &FT[0] ,32);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x02, &FT[1], 32);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x03, &FT[2], 32);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x04, &FT[3], 32);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x05, &FT[4], 32);
    conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x06, &FT[5], 32);
    FT[0]=-FT[0];FT[3]=-FT[3];

	// 打印电流 //
	auto &cout = controller->mout();
    if (target.count % 1 == 0)
	{
        for (int i = 0; i < 6; i++)
        {
            //cout << "pos" << i + 1 << ":" << target.model->motionPool()[i].mp() << "  ";
            //cout << "vel" << i + 1 << ":" << target.model->motionPool()[i].mv() << "  ";
            //cout << "cur" << i + 1 << ":" << target.model->motionPool()[i].ma() << "  ";
        }
     //   cout << target.count << "  ";
        //cout << std::endl;
	}

    auto &lout = controller->lout();

		for (int i = 0; i < 6; i++)
		{
            PositionList[6*(target.count-1)+i] = target.model->motionPool()[i].mp();
            SensorList[6*(target.count-1)+i] = FT[i];
		}



		    lout << target.count << ",";
			lout << PositionList[6 * (target.count - 1) + 0] << ",";lout << PositionList[6 * (target.count - 1) + 1] << ",";
			lout << PositionList[6 * (target.count - 1) + 2] << ",";lout << PositionList[6 * (target.count - 1) + 3] << ",";
			lout << PositionList[6 * (target.count - 1) + 4] << ",";lout << PositionList[6 * (target.count - 1) + 5] << ",";
			lout << SensorList[6 * (target.count - 1) + 0] << ",";lout << SensorList[6 * (target.count - 1) + 1] << ",";
			lout << SensorList[6 * (target.count - 1) + 2] << ",";lout << SensorList[6 * (target.count - 1) + 3] << ",";
			lout << SensorList[6 * (target.count - 1) + 4] << ",";lout << SensorList[6 * (target.count - 1) + 5] << ",";

			lout << std::endl;


    return SampleNum-target.count;
}

auto MoveDistal::collectNrt(aris::plan::PlanTarget &target)->void
{
  //  auto controller = target.controller;
   // auto &lout = controller->lout();
    std::cout<<"collect"<<std::endl;
    sixDistalMatrix.RLS(PositionList, SensorList, estParas,StatisError);

    for(int i=0;i<46;i++)
        cout<<estParas[i]<<",";

    std::cout<<endl;
    std::cout<<"*****************************Statictic Model Error*****************************************"<<std::endl;
    for(int i=0;i<6;i++)
        cout<<StatisError[i]<<endl;
    //double a = 3;
}


MoveDistal::MoveDistal(const std::string &name) :Plan(name)
{
	
	command().loadXmlStr(
		"<Command name=\"mvDistal\">"
		"	<GroupParam>"
        "		<Param name=\"period\"default=\"1.0\"/>"
        "		<Param name=\"amplitude\"default=\"0.2\"/>"
		"	</GroupParam>"
		"</Command>");

}



struct SetToolParam
{
	double period;
	double amplitude;

};
auto SetTool::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	SetToolParam param;

	for (auto &p : params)
	{
		if (p.first == "period")
			param.period = std::stod(p.second);
		if (p.first == "amplitude")
			param.amplitude = std::stod(p.second);

	}

	target.param = param;

	target.option |=
		Plan::USE_TARGET_POS |
		//#ifdef WIN32
		Plan::NOT_CHECK_POS_MIN |
		Plan::NOT_CHECK_POS_MAX |
		Plan::NOT_CHECK_POS_CONTINUOUS |
		Plan::NOT_CHECK_POS_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
		Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
		//#endif
		Plan::NOT_CHECK_VEL_MIN |
		Plan::NOT_CHECK_VEL_MAX |
		Plan::NOT_CHECK_VEL_CONTINUOUS |
		Plan::NOT_CHECK_VEL_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_VEL_FOLLOWING_ERROR;

	
	
}
auto SetTool::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<SetToolParam&>(target.param);


	double TransMatrix[6][12] = { { -0.6492,0.2770,0.7084,-0.2863,0.7738,-0.5650,-0.7046,-0.5697,-0.4231,1028.29,233.29,1432.98},
	{-0.8469, 0.1182, 0.5184, 0.4204, 0.7458, 0.5168, -0.3256, 0.6556, -0.6813,1118.69,-231.99,1478.85},
	{-0.8680, -0.0425, 0.4947, -0.0744, 0.9962, -0.0451, -0.4909, -0.0759, -0.8679,1129.45,25,1564},
	{-0.5835, 0.0035, 0.8121, 0.0020, 1.0000, -0.0028, -0.8121, 0.0000, -0.5835,983.98,0.39,1502.31},
	{ -0.5835, 0.0035, 0.8121, 0.0020, 1.0000, -0.0028, -0.8121, 0.0000, -0.5835,605.47,0.39,1502.31},
	{ -0.5835, 0.0035, 0.8121, 0.0020, 1.0000, -0.0028, -0.8121, 0.0000, -0.5835,983.98,0.39,1122.24}};
	double Atemp[5][9],Btemp[5][3];
	
	//target.model->generalMotionPool().at(0).getMpm(TransVector);
	for (int i = 0;i < 5;i++)
		for(int j=0;j<9;j++)
			Atemp[i][j] = TransMatrix[i][j] - TransMatrix[i+1][j];
	
	double L[3][3] = { 0 };
	for (int i = 0;i < 5;i++)
	{
		for (int m = 0;m < 3;m++)
			for (int n = 0;n < 3;n++)
				for (int j = 0;j < 3;j++)
				L[m][n] = L[m][n] + Atemp[i][j+3*m] * Atemp[i][j*3 + n];
	}

	for (int i = 0;i < 5;i++)
		for (int j = 0;j < 3;j++)
			Btemp[i][j] = TransMatrix[i + 1][j + 9] - TransMatrix[i][j + 9];

	double R[3] = { 0 };
	for (int i = 0;i < 5;i++)
	{
		for (int m = 0;m < 3;m++)
				for (int j = 0;j < 3;j++)
					R[m] = R[m] + Atemp[i][j + 3 * m] * Btemp[i][j];
	}

	// Pinv(L)*R,求取位置偏移Epos
	double Lvec[9];
	for(int i=0;i<3;i++)
		for (int j = 0;j < 3;j++)
			Lvec[i * 3 + j] = L[i][j];
		
	double U[9], tau[3], pinv[9], Epos[3];
	aris::Size p[3];
	aris::Size rank;

	// 根据 A 求出中间变量，相当于做 QR 分解 //
	// 请对 U 的对角线元素做处理
	s_householder_utp(3, 3, Lvec, U, tau, p, rank, 1e-10);
	
	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A) //
	double tau2[3];
	s_householder_utp2pinv(3, 3, rank, U, tau, p, pinv, tau2, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A)*b //
	s_mm(3, 1, 3, pinv, R, Epos);

	// 求取姿态偏移矩阵Epos
	double Trans4[9] = { -0.5835, 0.0035, 0.8121, 0.0020, 1.0000, -0.0028, -0.8121, 0.0000, -0.5835 };
	double Dis4[3] = { 983.98,0.39,1502.31 };
	double Dis5[3] = { 605.47,0.39,1502.31 };
	double Dis6[3] = { 983.98,0.39,1122.24 };
	double x1[3], x2[3],x12[3];
	s_mm(3, 1, 3, Trans4, Epos, x1);
	for (int i = 0;i < 3;i++)
		x1[i] = x1[i] + Dis4[i];
	s_mm(3, 1, 3, Trans4, Epos, x2);
	for (int i = 0;i < 3;i++)
		x2[i] = x2[i] + Dis5[i];

	//计算X方向的方向余弦
	double norm_x12 = 0,n_TE[3];
	for (int i = 0;i < 3;i++)
	{
        x12[i] = x2[i] - x1[i];
		norm_x12 = norm_x12 + x12[i] * x12[i];
	}
	norm_x12 = sqrt(norm_x12);
	for (int i = 0;i < 3;i++)
	{
		x12[i] = x12[i] / norm_x12;
	}
	// 根据 A 求出中间变量，相当于做 QR 分解 //
	s_householder_utp(3, 3, Trans4, U, tau, p, rank, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A) //
	s_householder_utp2pinv(3, 3, rank, U, tau, p, pinv, tau2, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A)*b //
	s_mm(3, 1, 3, pinv, x12, n_TE);

	//计算Z方向的方向余弦
	s_mm(3, 1, 3, Trans4, Epos, x2);
	for (int i = 0;i < 3;i++)
		x2[i] = x2[i] + Dis6[i];
	double a_TE[3];
	for (int i = 0;i < 3;i++)
	{
		x12[i] = x2[i] - x1[i];
		norm_x12 = norm_x12 + x12[i] * x12[i];
	}
	norm_x12 = sqrt(norm_x12);
	for (int i = 0;i < 3;i++)
	{
		x12[i] = x12[i] / norm_x12;
	}
	// 根据 A 求出中间变量，相当于做 QR 分解 //
	s_householder_utp(3, 3, Trans4, U, tau, p, rank, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A) //
	s_householder_utp2pinv(3, 3, rank, U, tau, p, pinv, tau2, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A)*b //
	s_mm(3, 1, 3, pinv, x12, a_TE);

	//计算Y方向的方向余弦
	double o_TE[3];
	crossVector(a_TE, n_TE, o_TE);
	return 10 - target.count;
}

auto SetTool::collectNrt(aris::plan::PlanTarget &target)->void
{

    //sixDistalMatrix.RLS(PositionList, SensorList, estParas);
	double a = 3;
}


SetTool::SetTool(const std::string &name) :Plan(name)
{
	
	command().loadXmlStr(
		"<Command name=\"STool\">"
		"	<GroupParam>"
		"		<Param name=\"period\"default=\"1.0\"/>"
		"		<Param name=\"amplitude\"default=\"0.2\"/>"
		"	</GroupParam>"
		"</Command>");
		

}







struct MovePressureParam
{
	double PressF;
	

};

auto MovePressure::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	MovePressureParam param;
	for (auto &p : params)
	{
		if (p.first == "PressF")
			param.PressF = std::stod(p.second);

	}

	target.param = param;

	target.option |=
		Plan::USE_TARGET_POS |
		//#ifdef WIN32
		Plan::NOT_CHECK_POS_MIN |
		Plan::NOT_CHECK_POS_MAX |
		Plan::NOT_CHECK_POS_CONTINUOUS |
		Plan::NOT_CHECK_POS_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
		Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
		Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
		//#endif
		Plan::NOT_CHECK_VEL_MIN |
		Plan::NOT_CHECK_VEL_MAX |
		Plan::NOT_CHECK_VEL_CONTINUOUS |
		Plan::NOT_CHECK_VEL_CONTINUOUS_AT_START |
		Plan::NOT_CHECK_VEL_FOLLOWING_ERROR;



   
}
auto MovePressure::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<MoveXYZParam&>(target.param);

	double RobotPosition[6];
	double RobotPositionJ[6];
	double RobotVelocity[6];
	double RobotAcceleration[6];
	double TorqueSensor[6];
	double X1[3];
	double X2[3];
	static double begin_pjs[6];
	static double step_pjs[6];
	static double stateTor0[6][3], stateTor1[6][3];
	static float FT0[6];

	// 访问主站 //
	auto controller = target.controller;

	// 获取当前起始点位置 //
	if (target.count == 1)
	{
		for (int i = 0; i < 6; ++i)
		{
			step_pjs[i] = target.model->motionPool()[i].mp();
			// controller->motionPool().at(i).setModeOfOperation(10);	//切换到电流控制
		}
	}


	if (!target.model->solverPool().at(1).kinPos())return -1;


	///* Using Jacobian, TransMatrix from ARIS
	double EndW[3], EndP[3], BaseV[3];
	double PqEnd[7], TransVector[16];
	target.model->generalMotionPool().at(0).getMpm(TransVector);
	target.model->generalMotionPool().at(0).getMpq(PqEnd);

	double dX[6] = { 0.00001, -0.0000, -0.0000, -0.0000, -0.0000, -0.0000 };
	double dTheta[6] = { 0 };

	float FT[6];
	uint16_t FTnum;
	auto conSensor = dynamic_cast<aris::control::EthercatController*>(target.controller);
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x00, &FTnum, 16);
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x01, &FT[0], 32);  //Fx
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x02, &FT[1], 32);  //Fy
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x03, &FT[2], 32);  //Fz
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x04, &FT[3], 32);
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x05, &FT[4], 32);
	conSensor->ecSlavePool().at(6).readPdo(0x6030, 0x06, &FT[5], 32);

	
	for (int i = 0;i < 6;i++)
	{
		RobotPositionJ[i] = target.model->motionPool()[i].mp();
		RobotPosition[i] = target.model->motionPool()[i].mp();
		RobotVelocity[i] = 0;
		RobotAcceleration[i] = 0;
		TorqueSensor[i] = FT[i];
	}


	// 获取当前起始点位置 //
	if (target.count == 1)
	{
		for (int j = 0; j < 6; j++)
		{
			stateTor0[j][0] = FT[j];
			FT0[j] = FT[j];
		}
	}

	

	for (int j = 0; j < 6; j++)
	{
		double A[3][3], B[3], CutFreq = 10;
		A[0][0] = 0; A[0][1] = 1; A[0][2] = 0;
		A[1][0] = 0; A[1][1] = 0; A[1][2] = 1;
		A[2][0] = -CutFreq * CutFreq * CutFreq;
		A[2][1] = -2 * CutFreq * CutFreq;
		A[2][2] = -2 * CutFreq;
		B[0] = 0; B[1] = 0;
		B[2] = -A[2][0];
		double intDT = 0.001;
		stateTor1[j][0] = stateTor0[j][0] + intDT * (A[0][0] * stateTor0[j][0] + A[0][1] * stateTor0[j][1] + A[0][2] * stateTor0[j][2] + B[0] * FT[j]);
		stateTor1[j][1] = stateTor0[j][1] + intDT * (A[1][0] * stateTor0[j][0] + A[1][1] * stateTor0[j][1] + A[1][2] * stateTor0[j][2] + B[1] * FT[j]);
		stateTor1[j][2] = stateTor0[j][2] + intDT * (A[2][0] * stateTor0[j][0] + A[2][1] * stateTor0[j][1] + A[2][2] * stateTor0[j][2] + B[2] * FT[j]);
	}


	for (int i = 0; i < 6; i++)
	{
		FT[i] = stateTor1[i][0] - FT0[i];
    }

    dX[2] = 1 * FT[2] / 80000;


	if (target.count % 100 == 0)
	{
		for (int i = 0; i < 6; i++)
		{
            cout << FT[i] << endl;

		}

		cout << std::endl;

	}


	for (int j = 0; j < 6; j++)
	{
		if (dX[j] > 0.00025)
			dX[j] = 0.00025;
		if (dX[j] < -0.00025)
			dX[j] = -0.00025;
	}


	// 打印电流 //
	auto &cout = controller->mout();

	// log 电流 //
	auto &lout = controller->lout();

	// lout << target.model->motionPool()[0].mp() << ",";
	 //lout << target.model->motionPool()[1].mp() << ",";
	 //lout << target.model->motionPool()[2].mp() << ",";
	 //lout << target.model->motionPool()[3].mp() << ",";
	 //lout << target.model->motionPool()[4].mp() << ",";
	 //lout << target.model->motionPool()[5].mp() << ",";


	lout << FT[1] << ",";lout << FT[2] << ",";
	lout << FT[3] << ",";lout << FT[4] << ",";
	lout << FT[5] << ",";lout << FT[6] << ",";
	lout << stateTor0[0][0] << ",";lout << stateTor0[1][0] << ",";
	lout << stateTor0[2][0] << ",";lout << stateTor0[3][0] << ",";
	lout << stateTor0[4][0] << ",";lout << stateTor0[5][0] << ",";

	//lout << stateTor1[2][0] << ",";lout << FT0[3] << ",";
   // lout << dX[0] << ",";

   // lout << dX[0] << ",";
   // lout << FT[1] << ",";lout << FT[2] << ",";
   // lout << FT[3] << ",";lout << FT[4] << ",";
   // lout << FT[5] << ",";lout << FT[6] << ",";
	lout << std::endl;



	///* Using Jacobian, TransMatrix from ARIS
	for (int i = 0;i < 3;i++)
		EndW[i] = dX[i + 3];

	for (int i = 0;i < 3;i++)
		EndP[i] = PqEnd[i];
	crossVector(EndP, EndW, BaseV);

	for (int i = 0;i < 3;i++)
		dX[i + 3] = dX[i + 3];
	for (int i = 0;i < 3;i++)
		dX[i] = dX[i] + BaseV[i];


	auto &fwd = dynamic_cast<aris::dynamic::ForwardKinematicSolver&>(target.model->solverPool()[1]);
	fwd.cptJacobi();
	double pinv[36];

	// 所需的中间变量，请对U的对角线元素做处理
	double U[36], tau[6];
	aris::Size p[6];
	aris::Size rank;

	// 根据 A 求出中间变量，相当于做 QR 分解 //
	// 请对 U 的对角线元素做处理
	s_householder_utp(6, 6, fwd.Jf(), U, tau, p, rank, 1e-10);
	for (int i = 0;i < 6;i++)
		if (U[7 * i] >= 0)
			U[7 * i] = U[7 * i] + 0.1;
		else
			U[7 * i] = U[7 * i] - 0.1;
	// 根据QR分解的结果求x，相当于Matlab中的 x = A\b //
	s_householder_utp_sov(6, 6, 1, rank, U, tau, p, dX, dTheta, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A) //
	double tau2[6];
	s_householder_utp2pinv(6, 6, rank, U, tau, p, pinv, tau2, 1e-10);

	// 根据QR分解的结果求广义逆，相当于Matlab中的 pinv(A)*b //
	s_mm(6, 1, 6, pinv, dX, dTheta);



	for (int i = 0; i < 6; i++)
	{
		if (dTheta[i] > 0.003)
			dTheta[i] = 0.003;
		if (dTheta[i] < -0.003)
			dTheta[i] = -0.003;
		//lout << dTheta[i] << ",";
	}


	//lout << std::endl;
	for (int i = 0; i < 6; i++)
	{
		dTheta[i] = dTheta[i] * DirectionFlag[i];

	}

	for (int i = 0; i < 6; i++)
	{
		step_pjs[i] = step_pjs[i] + dTheta[i];
        //target.model->motionPool().at(i).setMp(step_pjs[i]);
	}



	for (int i = 0; i < 6; i++)
	{

		stateTor0[i][0] = stateTor1[i][0];
		stateTor0[i][1] = stateTor1[i][1];
		stateTor0[i][2] = stateTor1[i][2];
	}

	return 150000000 - target.count;
   
}

MovePressure::MovePressure(const std::string &name) :Plan(name)
    {
	
        command().loadXmlStr(
			"<Command name=\"mvPre\">"
            "	<GroupParam>"
            "       <Param name=\"PressF\" default=\"0\"/>"
            "   </GroupParam>"
			"</Command>");
			
	

    }
