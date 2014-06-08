#include <iostream>

#include "TopicModel.h"

namespace stc{

namespace textMining{

TopicModel::TopicModel(size_t numberOfTopics, double alphaSum, double beta)
{
	this->numTopics = numberOfTopics;

	if (countOnes(numTopics) == 1) {
		// exact power of 2
		topicMask = numTopics - 1;
		topicBits = countOnes(topicMask);
	}
	else {
		// otherwise add an extra bit
		topicMask = highestOneBit(numTopics) * 2 - 1;
		topicBits = countOnes(topicMask);
	}

	this->alphaSum = alphaSum;
	alpha.resize(numberOfTopics);
	for (size_t ind = 0; ind < numberOfTopics; ++ind){ alpha[ind] = alphaSum/numberOfTopics;}

	this->beta = beta;

	tokensPerTopic.resize(numTopics);

	usingSymmetricAlpha = false;
	numIterations = 1000;
	burninPeriod = 200;
	saveSampleInterval = 10;
	optimizeInterval = 50;
	temperingInterval = 0;

	showTopicsInterval = 50;
	wordsPerTopic = 7;

	saveStateInterval = 0;

	saveModelInterval = 0;

	randomSeed = -1;
	printLogLikelihood = true;
	numThreads = 1;
}

unsigned int TopicModel::countOnes(unsigned int x)
{
	unsigned int numberOfOneBits = 0;

	while(x) {
		if ((x & 1) == 1) /* Is the least significant bit a 1? */
		  numberOfOneBits++;
		x >>= 1;          /* Shift number one bit to the right */
	}

	return numberOfOneBits;
}

unsigned int TopicModel::highestOneBit(unsigned int x)
{
	unsigned int cnt = 0;
	unsigned int indOfOneBit = 0;

	while(x) {
		if ((x & 1) == 1) /* Is the least significant bit a 1? */
		  indOfOneBit = cnt;
		x >>= 1;          /* Shift number one bit to the right */
		++ cnt;
	}

	return 1 << indOfOneBit;
}

TopicModel::~TopicModel()
{
	for (size_t ind = 0; ind < texts.size(); ++ ind){
		delete texts[ind];
	}
	texts.clear();
}

void TopicModel::setNumIterations (int numIterations) {
	this->numIterations = numIterations;
}

void TopicModel::setBurninPeriod (int burninPeriod) {
	this->burninPeriod = burninPeriod;
}

void TopicModel::setTopicDisplay(int interval, int n) {
	this->showTopicsInterval = interval;
	this->wordsPerTopic = n;
}

void TopicModel::setRandomSeed(unsigned int seed) {
	randomSeed = seed;
}

/** Interval for optimizing Dirichlet hyperparameters */
void TopicModel::setOptimizeInterval(int interval) {
	this->optimizeInterval = interval;

	// Make sure we always have at least one sample
	//  before optimizing hyperparameters
	if (saveSampleInterval > optimizeInterval) {
		saveSampleInterval = optimizeInterval;
	}
}

void TopicModel::setSymmetricAlpha(bool b) {
	usingSymmetricAlpha = b;
}

void TopicModel::setTemperingInterval(int interval) {
	temperingInterval = interval;
}

void TopicModel::setNumThreads(int threads) {
	this->numThreads = threads;
}

void TopicModel::setSaveState(int interval, std::string filename) {
	this->saveStateInterval = interval;
	this->stateFilename = filename;
}

void TopicModel::setSaveSerializedModel(int interval, std::string filename) {
	this->saveModelInterval = interval;
	this->modelFilename = filename;
}


void TopicModel::setModelOutput(int interval, std::string filename) {
	this->outputModelInterval = interval;
	this->outputModelFilename = filename;
}

void TopicModel::addDocuments(std::vector<std::string>& names, std::vector<std::vector<std::string>>& contents)
{
	try{
		size_t docs =names.size();
		for (size_t ind = 0; ind < docs; ++ind){
			std::vector<int> features(contents[ind].size());
			dictionary.addFeatures(contents[ind],features);
			texts.push_back(new Text(names[ind], features));
		}

		numTypes = dictionary.getFeaturesNum();
		typeTotals = dictionary.getFeatureTotals();
		typeTopicCounts.resize(numTypes);
		maxTypeCount = 0;

		for (size_t type = 0; type < numTypes; ++type) {
			if (typeTotals[type] > maxTypeCount) { maxTypeCount = typeTotals[type]; }
			typeTopicCounts[type].resize(std::min(numTopics, typeTotals[type]));
		}

		boost::random::mt19937 rng;
		rng.seed(randomSeed);
		boost::random::uniform_int_distribution<> uint_dist(0,numTopics-1);
		for (size_t ind = 0; ind < docs; ++ind){
			std::vector<int> topics = texts[ind]->getFeatureTopic();
			for (size_t position = 0; position < topics.size(); position++) {
				int topic = uint_dist(rng);
				topics[position] = topic;
			}
		}

		buildInitialTypeTopicCounts();
		initializeHistograms();
	} catch(...){
		std::cerr << "Can't load documents" << std::endl;

	}
}

void TopicModel::buildInitialTypeTopicCounts () {

	// Clear the topic totals
	for (size_t ind = 0; ind < tokensPerTopic.size(); ++ ind){
		tokensPerTopic[ind] = 0;
	}

	// Clear the type/topic counts, only
	//  looking at the entries before the first 0 entry.

	for (int type = 0; type < numTypes; type++) {

		std::vector<int>& topicCounts = typeTopicCounts[type];

		int position = 0;
		while (position < topicCounts.size() && topicCounts[position] > 0) {
			topicCounts[position] = 0;
			position++;
		}

	}

	for (size_t ind = 0; ind < texts.size(); ++ind){
		std::vector<int>& tokens = texts[ind]->getFeatures();
		std::vector<int>& topics = texts[ind]->getFeatureTopic();

		for (int position = 0; position < tokens.size(); position++) {

			int topic = topics[position];

			if (topic == UNASSIGNED_TOPIC) { continue; }

			tokensPerTopic[topic]++;

			// The format for these arrays is
			//  the topic in the rightmost bits
			//  the count in the remaining (left) bits.
			// Since the count is in the high bits, sorting (desc)
			//  by the numeric value of the int guarantees that
			//  higher counts will be before the lower counts.

			int type = tokens[position];
			std::vector<int>& currentTypeTopicCounts = typeTopicCounts[type];

			// Start by assuming that the array is either empty
			//  or is in sorted (descending) order.

			// Here we are only adding counts, so if we find
			//  an existing location with the topic, we only need
			//  to ensure that it is not larger than its left neighbor.

			int index = 0;
			int currentTopic = currentTypeTopicCounts[index] & topicMask;
			int currentValue;

			while (currentTypeTopicCounts[index] > 0 && currentTopic != topic) {
				index++;
				if (index == currentTypeTopicCounts.size()) {
					//throw("overflow on type " + type);
				}
				currentTopic = currentTypeTopicCounts[index] & topicMask;
			}
			currentValue = currentTypeTopicCounts[index] >> topicBits;

			if (currentValue == 0) {
				// new value is 1, so we don't have to worry about sorting
				//  (except by topic suffix, which doesn't matter)

				currentTypeTopicCounts[index] = (1 << topicBits) + topic;
			}
			else {
				currentTypeTopicCounts[index] = ((currentValue + 1) << topicBits) + topic;

				// Now ensure that the array is still sorted by
				//  bubbling this value up.
				while (index > 0 && currentTypeTopicCounts[index] > currentTypeTopicCounts[index - 1]) {
					int temp = currentTypeTopicCounts[index];
					currentTypeTopicCounts[index] = currentTypeTopicCounts[index - 1];
					currentTypeTopicCounts[index - 1] = temp;
					index--;
				}
			}
		}
	}
}


/**
	 *  Gather statistics on the size of documents
	 *  and create histograms for use in Dirichlet hyperparameter
	 *  optimization.
	 */
void TopicModel::initializeHistograms() {

	int maxTokens = 0;
	totalTokens = 0;
	int seqLen;

	for (int ind = 0; ind < texts.size(); ind++) {
		seqLen = texts[ind]->getFeatures().size();
		if (seqLen > maxTokens)
			maxTokens = seqLen;
		totalTokens += seqLen;
	}

	//logger.info("max tokens: " + maxTokens);
	//logger.info("total tokens: " + totalTokens);

	docLengthCounts.resize(maxTokens + 1);
	topicDocCounts.resize(numTopics);
	for (size_t ind = 0; ind < numTopics; ++ind){
		topicDocCounts[ind].resize(maxTokens + 1);
	}
}

void TopicModel::estimate()
{
	try{
		std::vector<WorkerRunnable> runnables;

		int docsPerThread = texts.size() / numThreads;
		int offset = 0;

		if (numThreads > 1) {

		}
		else {

			// If there is only one thread, copy the typeTopicCounts
			//  arrays directly, rather than allocating new memory.

			boost::random::mt19937 rng;
			rng.seed(randomSeed);

			runnables.push_back(WorkerRunnable(numTopics,
												alpha, alphaSum, beta,
												rng, texts,
												typeTopicCounts, tokensPerTopic,
												offset, docsPerThread));

			runnables[0].initializeAlphaStatistics(docLengthCounts.size());

			// If there is only one thread, we
			//  can avoid communications overhead.
			// This switch informs the thread not to
			//  gather statistics for its portion of the data.
			runnables[0].makeOnlyThread();
		}

		for (int iteration = 1; iteration <= numIterations; iteration++) {

			if (showTopicsInterval != 0 && iteration != 0 && iteration % showTopicsInterval == 0) {
				//logger.info("\n" + displayTopWords (wordsPerTopic, false));
			}

			if (saveStateInterval != 0 && iteration % saveStateInterval == 0) {
				//this.printState(new File(stateFilename + '.' + iteration));
			}

			if (saveModelInterval != 0 && iteration % saveModelInterval == 0) {
				//this.write(new File(modelFilename + '.' + iteration));
			}

			if (iteration > burninPeriod && optimizeInterval != 0 && iteration % saveSampleInterval == 0) {
				runnables[0].collectAlphaStatistics();
			}
			runnables[0].run();

			if (iteration > burninPeriod && optimizeInterval != 0 && iteration % optimizeInterval == 0) {

				optimizeAlpha(runnables);
				optimizeBeta(runnables);

			}
		}
	} catch(...){
		std::cerr << "Something wrong with estimator\n";
	}

}

void TopicModel::optimizeAlpha(std::vector<WorkerRunnable> runnables) {

	// First clear the sufficient statistic histograms

	for (size_t ind = 0; ind < docLengthCounts.size(); ++ ind){
		docLengthCounts[ind] = 0;
	}
	for (size_t topic = 0; topic < topicDocCounts.size(); topic++) {
		for (size_t ind = 0; ind < topicDocCounts[topic].size(); ++ ind){
			topicDocCounts[topic][ind] = 0;
		}
	}

	for (int thread = 0; thread < numThreads; thread++) {
		std::vector<int> sourceLengthCounts = runnables[thread].getDocLengthCounts();
		std::vector<std::vector<int> > sourceTopicCounts = runnables[thread].getTopicDocCounts();

		for (int count=0; count < sourceLengthCounts.size(); count++) {
			if (sourceLengthCounts[count] > 0) {
				docLengthCounts[count] += sourceLengthCounts[count];
				sourceLengthCounts[count] = 0;
			}
		}

		for (int topic=0; topic < numTopics; topic++) {

			if (! usingSymmetricAlpha) {
				for (int count=0; count < sourceTopicCounts[topic].size(); count++) {
					if (sourceTopicCounts[topic][count] > 0) {
						topicDocCounts[topic][count] += sourceTopicCounts[topic][count];
						sourceTopicCounts[topic][count] = 0;
					}
				}
			}
			else {
				// For the symmetric version, we only need one
				//  count array, which I'm putting in the same
				//  data structure, but for topic 0. All other
				//  topic histograms will be empty.
				// I'm duplicating this for loop, which
				//  isn't the best thing, but it means only checking
				//  whether we are symmetric or not numTopics times,
				//  instead of numTopics * longest document length.
				for (int count=0; count < sourceTopicCounts[topic].size(); count++) {
					if (sourceTopicCounts[topic][count] > 0) {
						topicDocCounts[0][count] += sourceTopicCounts[topic][count];
						//			 ^ the only change
						sourceTopicCounts[topic][count] = 0;
					}
				}
			}
		}
	}

	if (usingSymmetricAlpha) {
		alphaSum = Dirichlet::learnSymmetricConcentration(topicDocCounts[0], docLengthCounts, numTopics, alphaSum);
		for (int topic = 0; topic < numTopics; topic++) {
			alpha[topic] = alphaSum / numTopics;
		}
	}
	else {
		alphaSum = Dirichlet::learnParameters(alpha, topicDocCounts, docLengthCounts, 1.001, 1.0, 1);
	}
}

void TopicModel::optimizeBeta(std::vector<WorkerRunnable>& runnables)
{
	// The histogram starts at count 0, so if all of the
	//  tokens of the most frequent type were assigned to one topic,
	//  we would need to store a maxTypeCount + 1 count.
	std::vector<int> countHistogram(maxTypeCount + 1);

	// Now count the number of type/topic pairs that have
	//  each number of tokens.

	int index;
	for (int type = 0; type < numTypes; type++) {
		std::vector<int> counts = typeTopicCounts[type];
		index = 0;
		while (index < counts.size() && counts[index] > 0) {
			int count = counts[index] >> topicBits;
			countHistogram[count]++;
			index++;
		}
	}

	// Figure out how large we need to make the "observation lengths"
	//  histogram.
	int maxTopicSize = 0;
	for (int topic = 0; topic < numTopics; topic++) {
		if (tokensPerTopic[topic] > maxTopicSize) {
			maxTopicSize = tokensPerTopic[topic];
		}
	}

	// Now allocate it and populate it.
	std::vector<int> topicSizeHistogram(maxTopicSize + 1);
	for (int topic = 0; topic < numTopics; topic++) {
		topicSizeHistogram[ tokensPerTopic[topic] ]++;
	}

	betaSum = Dirichlet::learnSymmetricConcentration(countHistogram, topicSizeHistogram, numTypes, betaSum);
	beta = betaSum / numTypes;


	//logger.info("[beta: " + formatter.format(beta) + "] ");
	// Now publish the new value
	for (int thread = 0; thread < numThreads; thread++) {
		runnables[thread].resetBeta(beta, betaSum);
	}

}


bool compare(std::pair<int, double>& p1, std::pair<int, double>& p2)
{
	return p1.second > p2.second;
}

std::vector<std::vector<std::pair<int, double> > > TopicModel::getSortedWords()
{

	std::vector<std::vector<std::pair<int, double> > > topicSortedWords(numTopics);

	// Collect counts
	for (int type = 0; type < numTypes; type++) {

		std::vector<int>& topicCounts = typeTopicCounts[type];

		int index = 0;
		while (index < topicCounts.size() && topicCounts[index] > 0) {

			int topic = topicCounts[index] & topicMask;
			int count = topicCounts[index] >> topicBits;

			topicSortedWords[topic].push_back(std::pair<int, double>(type, count));

			index++;
		}
	}

	for (size_t topic = 0; topic < numTopics; ++ topic){
		std::sort(topicSortedWords[topic].begin(), topicSortedWords[topic].end(), compare);
	}

	return topicSortedWords;
}

void TopicModel::getDocumentTopics(double threshold, int max, std::map<std::string,std::vector<int> > & res)
{
	try{
		int docLen;
		std::vector<int> topicCounts(numTopics);

		std::vector<std::pair<int, double> > sortedTopics(numTopics);

		if (max <= 0 || max > numTopics) {
			max = numTopics;
		}
		for (int doc = 0; doc < texts.size(); doc++) {
			std::vector<int>& currentDocTopics = texts[doc]->getFeatureTopic();

			docLen = currentDocTopics.size();

			if (texts[doc]->getName() != "") {
				res[texts[doc]->getName()].reserve(1);
			}
			else {
				res["no-name"].reserve(1);
			}

			// Count up the tokens
			for (int token = 0; token < docLen; token++) {
				topicCounts[currentDocTopics[token]]++;
			}

			// And normalize
			for (int topic = 0; topic < numTopics; topic++) {
				sortedTopics[topic].first = topic;
				sortedTopics[topic].second = (alpha[topic] + topicCounts[topic]) / (docLen + alphaSum) ;
			}

			std::sort(sortedTopics.begin(), sortedTopics.end(), compare);

			for (int i = 0; i < max; i++) {
				if (sortedTopics[i].second < threshold) { break; }
				res[texts[doc]->getName()].push_back(sortedTopics[i].first);
			}

//			for (int token = 0; token < numTopics; token++) {
//				topicCounts[token] = 0;
//			}
                        topicCounts.assign(numTopics, 0);
		}
	} catch(...){
		std::cerr << "Can't display topic per document distribution\n";
	}

}

std::string TopicModel::displayTopWords(int numWords, bool usingNewLines)
{
	std::string out = "";
	try{
		std::vector<std::vector<std::pair<int, double> >> topicSortedWords = getSortedWords();

		// Print results for each topic
		for (int topic = 0; topic < numTopics; topic++) {
			std::vector<std::pair<int, double> >& sortedWords = topicSortedWords[topic];
			int word = 1;
			std::vector<std::pair<int, double> >::iterator it = sortedWords.begin();

			if (usingNewLines) {
				out += std::to_string((long long)topic) + "\t" + std::to_string((long long)alpha[topic]) + "\n";
				while (it != sortedWords.end() && word <= numWords) {
					std::pair<int, double> info = *it;
					out += dictionary.getFeatureDecoded(info.first) + "\t" + std::to_string((long long)info.second) + "\n";
					word++;
					it++;
				}
			}
			else {
				out += std::to_string((long long)topic) + "\t" + std::to_string((long long)alpha[topic]) + "\n";

				while (it != sortedWords.end() && word <= numWords) {
					std::pair<int, double> info = *it;
					out += dictionary.getFeatureDecoded(info.first) + " ";
					word++;
					it++;
				}
				out.append ("\n");
			}
                        out.append ("\n");
		}
	} catch(...){
		std::cerr << "Can't display top words\n";
	}

	return out;
}



void TopicModel::displayDocumentTopics(std::ofstream & out,double threshold, int max)
{
	try{
		out << "#doc name topic proportion ...\n";
		int docLen;
		std::vector<int> topicCounts(numTopics);

		std::vector<std::pair<int, double> > sortedTopics(numTopics);

		if (max <= 0 || max > numTopics) {
			max = numTopics;
		}

		for (int doc = 0; doc < texts.size(); doc++) {
			std::vector<int>& currentDocTopics = texts[doc]->getFeatureTopic();

			out << doc << "\t";

			if (texts[doc]->getName() != "") {
				out << texts[doc]->getName();
			}
			else {
				out << "no-name";
			}

			out << "\t";
			docLen = currentDocTopics.size();

			// Count up the tokens
			for (int token = 0; token < docLen; token++) {
				topicCounts[currentDocTopics[token]]++;
			}

			// And normalize
			for (int topic = 0; topic < numTopics; topic++) {
				sortedTopics[topic].first = topic;
				sortedTopics[topic].second = (alpha[topic] + topicCounts[topic]) / (docLen + alphaSum) ;
			}

			std::sort(sortedTopics.begin(), sortedTopics.end(), compare);

			for (int i = 0; i < max; i++) {
				if (sortedTopics[i].second < threshold) { break; }

				out << sortedTopics[i].first << "\t"  << sortedTopics[i].second << "\t";
			}
			out << std::endl;
//			for (int token = 0; token < numTopics; token++) {
//				topicCounts[token] = 0;
//			}
                        topicCounts.assign(numTopics, 0);
		}
	} catch(...){
		std::cerr << "Can't display topic per document distribution\n";
	}

}
}
}