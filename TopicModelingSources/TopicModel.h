#include <string>
#include <cstdlib>
#include <vector>
#include <list>
#include <time.h>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "Topic.h"
#include "WorkerRunnable.h"
#include "Dirichlet.h"


#define DEFAULT_BETA 0.01
#define UNASSIGNED_TOPIC -1

namespace stc{
namespace textMining{

class TopicModel
{
public:
    TopicModel(size_t numberOfTopics, double alphaSum, double beta);
    ~TopicModel();

    void setNumIterations (int numIterations);

    void setBurninPeriod (int burninPeriod);

    void setTopicDisplay(int interval, int n);

    void setRandomSeed(unsigned int seed);

    void setOptimizeInterval(int interval);

    void setSymmetricAlpha(bool b);

    void setTemperingInterval(int interval);

    void setNumThreads(int threads);

    void setSaveState(int interval, std::string filename);

    void setSaveSerializedModel(int interval, std::string filename);

    void setModelOutput(int interval, std::string filename);

    void buildInitialTypeTopicCounts();

    void addDocuments(std::vector<std::string>& names, std::vector<std::vector<std::string>>& contents);

    void optimizeAlpha(std::vector<WorkerRunnable> runnables);

    void optimizeBeta(std::vector<WorkerRunnable>& runnables);

    void fillDocumentTopicDistribution();

    void estimate();

    const std::vector<int> & getCountTokensPerTopic(size_t topic) const;

    void displayTopWords(std::ofstream & out, size_t numWords = 20, bool usingNewLines = true);

    void displayDocumentTopics(std::ofstream& out, double threshold = 0, int max = -1);


private:

    unsigned int countOnes(unsigned int x);
    unsigned int highestOneBit(unsigned int x);

    std::vector<std::vector<std::pair<int, double> > > getSortedWords();
    void initializeHistograms();

    Dictionary dictionary;

    std::vector<Text *> texts;
    int numTopics; // Number of topics to be fit

    // These values are used to encode type/topic counts as
    //  count/topic pairs in a single int.
    int topicMask;
    int topicBits;

    int numTypes;
    int totalTokens;

    std::vector<double> alpha;	 // Dirichlet(alpha,alpha,...) is the distribution over topics
    double alphaSum;
    double beta;   // Prior on per-topic multinomial distribution over words
    double betaSum;

    bool usingSymmetricAlpha;

    std::vector<std::vector<int>> typeTopicCounts; // indexed by <feature index, topic index>
    std::vector<int> tokensPerTopic; // indexed by <topic index>

    // for dirichlet estimation
    std::vector<int> docLengthCounts; // histogram of document sizes
    std::vector<std::vector<int> > topicDocCounts; // histogram of document/topic counts, indexed by <topic index, sequence position index>

    int numIterations;
    int burninPeriod;
    int saveSampleInterval;
    int optimizeInterval;
    int temperingInterval;

    int showTopicsInterval;
    int wordsPerTopic;

    int saveStateInterval;
    std::string stateFilename;

    int saveModelInterval;
    std::string modelFilename;

    unsigned int randomSeed;
    bool printLogLikelihood;

    // The number of times each type appears in the corpus
    std::vector<int> typeTotals;
    // The max over typeTotals, used for beta optimization
    int maxTypeCount;

    int outputModelInterval;

    std::string outputModelFilename;

    int numThreads;

};

}
}