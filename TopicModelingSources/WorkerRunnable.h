#include <cstdlib>
#include <vector>
#include <iostream>
#include <random>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include "Text.h"

#define DEFAULT_BETA  0.01
#define UNASSIGNED_TOPIC -1

namespace stc{

namespace textMining{

class WorkerRunnable
{
public:
	WorkerRunnable(int numTopics, std::vector<double>& alpha, double alphaSum, double beta,
							   boost::random::mt19937 & rng, std::vector<Text *>& texts,
							   std::vector<std::vector<int> >& typeTopicCounts, 
							   std::vector<int>& tokensPerTopic,
							   int startDoc, int numDocs);
	~WorkerRunnable();

	void makeOnlyThread();

	std::vector<int>& getTokensPerTopic() { return tokensPerTopic; }
	std::vector<std::vector<int> >& getTypeTopicCounts() { return typeTopicCounts; }

	std::vector<int>& getDocLengthCounts() { return docLengthCounts; }
	std::vector<std::vector<int> >& getTopicDocCounts() { return topicDocCounts; }

	void initializeAlphaStatistics(int size);
	
	void collectAlphaStatistics();

	void resetBeta(double beta, double betaSum);

	void run();

protected:

	void sampleTopicsForOneDoc (std::vector<int>& tokenSequence, std::vector<int>& topicSequence,
										  bool readjustTopicsAndStats /* currently ignored */);

	void buildLocalTypeTopicCounts();

	bool isFinished;

	std::vector<Text *>& texts;
	int startDoc, numDocs;

	int numTopics; // Number of topics to be fit

	// These values are used to encode type/topic counts as
	//  count/topic pairs in a single int.
	int topicMask;
	int topicBits;

	int numTypes;

	std::vector<double>& alpha;	 // Dirichlet(alpha,alpha,...) is the distribution over topics
	double alphaSum;
	double beta;   // Prior on per-topic multinomial distribution over words
	double betaSum;
	
	double smoothingOnlyMass;
	std::vector<double> cachedCoefficients;

	std::vector<std::vector<int> >& typeTopicCounts; // indexed by <feature index, topic index>
	std::vector<int>& tokensPerTopic; // indexed by <topic index>

	// for dirichlet estimation
	std::vector<int> docLengthCounts; // histogram of document sizes
	std::vector<std::vector<int> > topicDocCounts; // histogram of document/topic counts, indexed by <topic index, sequence position index>

	bool shouldSaveState;
	bool shouldBuildLocalCounts;
	
	boost::random::mt19937 rng;

private:
	//TODO change destination of these functions
	unsigned int countOnes(unsigned int x);
	unsigned int highestOneBit(unsigned int x);

};

}
}