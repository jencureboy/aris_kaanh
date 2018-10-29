#ifndef IIR_H_
#define IIR_H_

#include <vector>

# define FORE_VEL_LENGTH 20	//速度平均值滤波buffer长度

namespace IIR_FILTER
{ 
	class IIR
	{
		private:
			std::vector<double> m_pNum;
			std::vector<double> m_pDen;
			std::vector<double> m_px;
			std::vector<double> m_py;
		public:
			int m_num_order = 10;
			int m_den_order = 10;
			IIR();
			void IIR::setPara(std::vector<double> &num, std::vector<double> &den);
			double filter(double data);
	};
	//IIR滤波器设计参数//
	const double num[] = { 0.000000046264715245537615778738675838266,
					0.000000416382437209838515538868480847512,
					0.000001665529748839354273913710736965132,
					0.000003886236080625160043051403990777004,
					0.000005829354120937740064577105986165506,
					0.000005829354120937740064577105986165506,
					0.000003886236080625160043051403990777004,
					0.000001665529748839354273913710736965132,
					0.000000416382437209838515538868480847512,
					0.000000046264715245537615778738675838266 };
	const double den[] = { 1,
					 - 7.05312858422665289737096827593632042408,
					 22.281832075423505301614568452350795269012,
					- 41.353698036863043796529382234439253807068,
					 49.662158421419455578416091157123446464539,
					- 40.000995187899327731884113745763897895813,
					 21.600811941456669273975421674549579620361,
					 - 7.538271742701063260483351768925786018372,
					  1.542194756274490874403682028059847652912,
					 - 0.140879955349824004251502174156485125422 };
	
}

#endif