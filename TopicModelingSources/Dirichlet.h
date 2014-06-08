#include <cstdlib>
#include <vector>
#include <random>
#include <cmath>

#include "Dictionary.h"

namespace stc{

namespace textMining{


class Dirichlet
{
public:
	Dirichlet(double m, std::vector<double>& p);
	Dirichlet(std::vector<double>& p);
	Dirichlet(std::vector<double>& alphas, Dictionary& dict);
	Dirichlet(Dictionary& dict);
	Dirichlet(Dictionary& dict, double alpha = 1.0);
	Dirichlet(int size);
	Dirichlet(int size, double alpha = 1.0);
	~Dirichlet();

	static double learnParameters(std::vector<double>& parameters, std::vector<std::vector<int> >& observations, std::vector<int>& observationLengths, 
									 double shape = 1.00001, double scale =1.0, int numIterations = 200);

	static double learnSymmetricConcentration(std::vector<int>& countHistogram, std::vector<int>& observationLengths,
													 int numDimensions, double currentValue);
	static double digamma(double z);

	Dictionary dict;
	double magnitude;
	std::vector<double> partition;

	std::mt19937 rng;

	/** Actually the negative Euler-Mascheroni constant */
	static const  double EULER_MASCHERONI;
	static const  double PI_SQUARED_OVER_SIX;
	static const  double HALF_LOG_TWO_PI;

	static const  double DIGAMMA_COEF_1;
	static const  double DIGAMMA_COEF_2;
	static const  double DIGAMMA_COEF_3;
	static const  double DIGAMMA_COEF_4;
	static const  double DIGAMMA_COEF_5;
	static const  double DIGAMMA_COEF_6;
	static const  double DIGAMMA_COEF_7;
	static const  double DIGAMMA_COEF_8;
	static const  double DIGAMMA_COEF_9;
	static const  double DIGAMMA_COEF_10;

	static const  double DIGAMMA_LARGE;
	static const  double DIGAMMA_SMALL;
private:

};

}
}