#include "Dirichlet.h"

namespace stc{

namespace textMining{
const double Dirichlet::EULER_MASCHERONI = -0.5772156649015328606065121;
const double Dirichlet::PI_SQUARED_OVER_SIX = acos((double)-1) * acos((double)-1) / 6;
const double Dirichlet::HALF_LOG_TWO_PI = std::log(2 * acos((double)-1)) / 2;

const double Dirichlet::DIGAMMA_COEF_1 = 1/12;
const double Dirichlet::DIGAMMA_COEF_2 = 1/120;
const double Dirichlet::DIGAMMA_COEF_3 = 1/252;
const double Dirichlet::DIGAMMA_COEF_4 = 1/240;
const double Dirichlet::DIGAMMA_COEF_5 = 1/132;
const double Dirichlet::DIGAMMA_COEF_6 = 691/32760;
const double Dirichlet::DIGAMMA_COEF_7 = 1/12;
const double Dirichlet::DIGAMMA_COEF_8 = 3617/8160;
const double Dirichlet::DIGAMMA_COEF_9 = 43867/14364;
const double Dirichlet::DIGAMMA_COEF_10 = 174611/6600;

const double Dirichlet::DIGAMMA_LARGE = 9.5;
const double Dirichlet::DIGAMMA_SMALL = .000001;

Dirichlet::Dirichlet(double m, std::vector<double>& p)
{
	magnitude = m;
	partition = p;
}

Dirichlet::Dirichlet(std::vector<double>& p)
{
	magnitude = 0;
	partition.resize(p.size());

	// Add up the total
	for (size_t i = 0; i < p.size(); ++ i) {
		magnitude += p[i];
	}

	for (size_t i = 0; i < p.size(); ++ i) {
		partition[i] = p[i] / magnitude;
	}
}


Dirichlet::Dirichlet(std::vector<double>& alphas, Dictionary& dict)
{
	magnitude = 0;
	partition.resize(alphas.size());

	// Add up the total
	for (size_t i = 0; i < alphas.size(); ++ i) {
		magnitude += alphas[i];
	}

	for (size_t i = 0; i < alphas.size(); ++ i) {
		partition[i] = alphas[i] / magnitude;
	}

	//if (dict  && alphas.size() != dict.size())
	//	throw new IllegalArgumentException ("alphas and dict sizes do not match.");
	this->dict = dict;
	dict.stopGrowth();
}

/**
	*	A symmetric Dirichlet with alpha_i = <code>alpha</code> and the 
	*	number of dimensions of the given alphabet.
	*/
Dirichlet::Dirichlet(Dictionary& dict, double alpha)
{
	int size = dict.getFeaturesNum();
	magnitude = size * alpha;

	partition.resize(size);

	partition[0] = 1.0 / size;
	for (size_t i = 1; i < size; i++) {
		partition[i] = partition[0];
	}
	this->dict = dict;
	dict.stopGrowth();
}


/** A symmetric dirichlet: E(X_i) = E(X_j) for all i, j 
* 
* @param n The number of dimensions
* @param alpha The parameter for each dimension
*/
Dirichlet::Dirichlet(int size, double alpha)
{
	magnitude = size * alpha;

	partition.resize(size);

	partition[0] = 1.0 / size;
	for (size_t i = 1; i < size; i++) {
		partition[i] = partition[0];
	}
}

Dirichlet::~Dirichlet()
{
}


double Dirichlet::learnSymmetricConcentration(std::vector<int>& countHistogram, std::vector<int>& observationLengths,
													 int numDimensions, double currentValue)
{
	double currentDigamma;

	// The histogram arrays are presumably allocated before
	//  we knew what went in them. It is therefore likely that
	//  the largest non-zero value may be much closer to the 
	//  beginning than the end. We don't want to iterate over
	//  a whole bunch of zeros, so keep track of the last value.
	int largestNonZeroCount = 0;
	std::vector<int> nonZeroLengthIndex(observationLengths.size());
		
	for (int index = 0; index < countHistogram.size(); index++) {
		if (countHistogram[index] > 0) { largestNonZeroCount = index; }
	}

	int denseIndex = 0;
	for (int index = 0; index < observationLengths.size(); index++) {
		if (observationLengths[index] > 0) {
			nonZeroLengthIndex[denseIndex] = index;
			denseIndex++;
		}
	}

	int denseIndexSize = denseIndex;

	for (int iteration = 1; iteration <= 200; iteration++) {
			
		double currentParameter = currentValue / numDimensions;

		// Calculate the numerator
			
		currentDigamma = 0;
		double numerator = 0;
		
		// Counts of 0 don't matter, so start with 1
		for (int index = 1; index <= largestNonZeroCount; index++) {
			currentDigamma += 1.0 / (currentParameter + index - 1);
			numerator += countHistogram[index] * currentDigamma;
		}
			
		// Now calculate the denominator, a sum over all observation lengths
			
		currentDigamma = 0;
		double denominator = 0;
		int previousLength = 0;
			
		double cachedDigamma = digamma(currentValue);
			
		for (denseIndex = 0; denseIndex < denseIndexSize; denseIndex++) {
			int length = nonZeroLengthIndex[denseIndex];
				
			if (length - previousLength > 20) {
				// If the next length is sufficiently far from the previous,
				//  it's faster to recalculate from scratch.
				currentDigamma = digamma(currentValue + length) - cachedDigamma;
			}
			else {
				// Otherwise iterate up. This looks slightly different
				//  from the previous version (no -1) because we're indexing differently.
				for (int index = previousLength; index < length; index++) {
					currentDigamma += 1.0 / (currentValue + index);
				}
			}
				
			denominator += currentDigamma * observationLengths[length];
		}
			
		currentValue = currentParameter * numerator / denominator;


		///System.out.println(currentValue + " = " + currentParameter + " * " + numerator + " / " + denominator);
	}

	return currentValue;
}



/** Calculate digamma using an asymptotic expansion involving
Bernoulli numbers. */
double Dirichlet::digamma(double z)
{
//		This is based on matlab code by Tom Minka

//		if (z < 0) { System.out.println(" less than zero"); }

	double psi = 0;

	if (z < DIGAMMA_SMALL) {
		psi = EULER_MASCHERONI - (1 / z); // + (PI_SQUARED_OVER_SIX * z);
		/*for (int n=1; n<100000; n++) {
psi += z / (n * (n + z));
}*/
		return psi;
	}

	while (z < DIGAMMA_LARGE) {
		psi -= 1 / z;
		z++;
	}

	double invZ = 1/z;
	double invZSquared = invZ * invZ;

	psi += std::log(z) - .5 * invZ
	- invZSquared * (DIGAMMA_COEF_1 - invZSquared * 
			(DIGAMMA_COEF_2 - invZSquared * 
					(DIGAMMA_COEF_3 - invZSquared * 
							(DIGAMMA_COEF_4 - invZSquared * 
									(DIGAMMA_COEF_5 - invZSquared * 
											(DIGAMMA_COEF_6 - invZSquared *
													DIGAMMA_COEF_7))))));

	return psi;
}


double Dirichlet::learnParameters(std::vector<double>& parameters, std::vector<std::vector<int> >& observations, std::vector<int>& observationLengths, 
							  double shape, double scale, int numIterations) 
{
	int i, k;

	double parametersSum = 0;

	//	Initialize the parameter sum

	for (k = 0; k < parameters.size(); ++ k) {
		parametersSum += parameters[k];
	}

	double oldParametersK;
	double currentDigamma;
	double denominator;

	int nonZeroLimit;
	std::vector<int> nonZeroLimits(observations.size());
	for (size_t ind = 0; ind < nonZeroLimits.size(); ++ ind){
		nonZeroLimits[ind] = -1;
	}

	// The histogram arrays go up to the size of the largest document,
	//	but the non-zero values will almost always cluster in the low end.
	//	We avoid looping over empty arrays by saving the index of the largest
	//	non-zero value.

	std::vector<int> histogram;

	for (i = 0; i < observations.size(); i++) {
		histogram = observations[i];

		//StringBuffer out = new StringBuffer();
		for (k = 0; k < histogram.size(); k++) {
			if (histogram[k] > 0) {
				nonZeroLimits[i] = k;
				//out.append(k + ":" + histogram[k] + " ");
			}
		}
		//System.out.println(out);
	}

	for (int iteration=0; iteration<numIterations; iteration++) {

		// Calculate the denominator
		denominator = 0;
		currentDigamma = 0;

		// Iterate over the histogram:
		for (i=1; i<observationLengths.size(); i++) {
			currentDigamma += 1 / (parametersSum + i - 1);
			denominator += observationLengths[i] * currentDigamma;
		}

		// Bayesian estimation Part I
		denominator -= 1/scale;

		// Calculate the individual parameters

		parametersSum = 0;
			
		for (k = 0; k < parameters.size(); k++) {

			// What's the largest non-zero element in the histogram?
			nonZeroLimit = nonZeroLimits[k];

			oldParametersK = parameters[k];
			parameters[k] = 0;
			currentDigamma = 0;

			histogram = observations[k];

			for (i = 1; i <= nonZeroLimit; i++) {
				currentDigamma += 1 / (oldParametersK + i - 1);
				parameters[k] += histogram[i] * currentDigamma;
			}

			// Bayesian estimation part II
			parameters[k] = (oldParametersK * parameters[k] + shape) / denominator;

			parametersSum += parameters[k];
		}
	}

	return parametersSum;
}
}
}