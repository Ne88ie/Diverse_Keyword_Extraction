#include "WorkerRunnable.h"

namespace stc{

namespace textMining{

WorkerRunnable::WorkerRunnable(int numTopics, std::vector<double>& alpha, double alphaSum, double beta,
							   boost::random::mt19937& rng,std::vector<Text *>& texts,
							   std::vector<std::vector<int> >& typeTopicCounts, 
							   std::vector<int>& tokensPerTopic,
							   int startDoc, int numDocs):
numTopics(numTopics), alpha(alpha), texts(texts), typeTopicCounts(typeTopicCounts), tokensPerTopic(tokensPerTopic)
{
	smoothingOnlyMass = 0.0;
	shouldSaveState = false;
	shouldBuildLocalCounts = true;
	isFinished = true;

	this->numTypes = typeTopicCounts.size();

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
	this->beta = beta;
	this->betaSum = beta * numTypes;
		
	this->startDoc = startDoc;
	this->numDocs = numDocs;
	this->rng = rng;

	cachedCoefficients.resize(numTopics);
}

unsigned int WorkerRunnable::countOnes(unsigned int x)
{
	unsigned int numberOfOneBits = 0;

	while(x) {
		if ((x & 1) == 1){ /* Is the least significant bit a 1? */
		  numberOfOneBits++;
		}
		x >>= 1;          /* Shift number one bit to the right */
	}

	return numberOfOneBits; 
}

unsigned int WorkerRunnable::highestOneBit(unsigned int x)
{
	unsigned int cnt = 0;
	unsigned int indOfOneBit = 0;

	while(x) {
		if ((x & 1) == 1){ /* Is the least significant bit a 1? */
		  indOfOneBit = cnt;
		}
		x >>= 1;          /* Shift number one bit to the right */
		++ cnt;
	}

	return 1 << indOfOneBit; 
}

WorkerRunnable::~WorkerRunnable()
{
}

void WorkerRunnable::makeOnlyThread() 
{
	shouldBuildLocalCounts = false;
}

void WorkerRunnable::initializeAlphaStatistics(int size) 
{
	docLengthCounts.resize(size);
	topicDocCounts.resize(numTopics);
	for (size_t ind = 0; ind < numTopics; ++ind){
		topicDocCounts[ind].resize(size);
	}
}
	
void WorkerRunnable::collectAlphaStatistics() 
{
	shouldSaveState = true;
}

void WorkerRunnable::resetBeta(double beta, double betaSum) 
{
	this->beta = beta;
	this->betaSum = betaSum;
}


void WorkerRunnable::run() 
{
	try {
			
		if (! isFinished) { std::cout << "already running!" ; return; }
			
		isFinished = false;
			
		// Initialize the smoothing-only sampling bucket
		smoothingOnlyMass = 0;
			
		// Initialize the cached coefficients, using only smoothing.
		//  These values will be selectively replaced in documents with
		//  non-zero counts in particular topics.
			
		for (size_t topic = 0; topic < numTopics; topic++) {
			smoothingOnlyMass += alpha[topic] * beta / (tokensPerTopic[topic] + betaSum);
			cachedCoefficients[topic] =  alpha[topic] / (tokensPerTopic[topic] + betaSum);
		}
		for (size_t doc = startDoc; doc < texts.size() && doc < startDoc + numDocs; doc++) {
			std::vector<int>& tokens = texts[doc]->getFeatures();
			std::vector<int>& topics = texts[doc]->getFeatureTopic();
			/*
				
			if (doc % 10 == 0) {
				std::cout << "   processing doc " << doc << std::endl;
				for (size_t topic = 0; topic < numTopics; topic++) {
					if (topics.size() > topic){
						std::cout << tokensPerTopic[topic] << " " << topics[topic] << "; ";
					}else{
						std::cout << tokensPerTopic[topic] << "; ";
					}
				}
				std::cout << std::endl;	
			}	
			*/	
			sampleTopicsForOneDoc(tokens, topics, true);
			
		}
			
		if (shouldBuildLocalCounts) {
			buildLocalTypeTopicCounts();
		}

		shouldSaveState = false;
		isFinished = true;

	} catch (...) {
		std::cout << "oops!" ; return;
	}
}

void WorkerRunnable::sampleTopicsForOneDoc (std::vector<int>& tokenSequence, std::vector<int>& topicSequence,
										  bool readjustTopicsAndStats /* currently ignored */) 
{
	std::vector<int>& oneDocTopics = topicSequence;

	int type, oldTopic, newTopic;
	double topicWeightsSum;
	int docLength = tokenSequence.size();

	std::vector<int> localTopicCounts(numTopics);
	std::vector<int> localTopicIndex(numTopics);

	//		populate topic counts
	for (int position = 0; position < docLength; position++) {
		if (oneDocTopics[position] == UNASSIGNED_TOPIC) { continue; }
		localTopicCounts[oneDocTopics[position]]++;
	}

	// Build an array that densely lists the topics that
	//  have non-zero counts.
	int denseIndex = 0;
	for (int topic = 0; topic < numTopics; topic++) {
		if (localTopicCounts[topic] != 0) {
			localTopicIndex[denseIndex] = topic;
			denseIndex++;
		}
	}

	// Record the total number of non-zero topics
	int nonZeroTopics = denseIndex;

	//		Initialize the topic count/beta sampling bucket
	double topicBetaMass = 0.0;

	// Initialize cached coefficients and the topic/beta 
	//  normalizing constant.

	for (denseIndex = 0; denseIndex < nonZeroTopics; denseIndex++) {
		int topic = localTopicIndex[denseIndex];
		int n = localTopicCounts[topic];

		//	initialize the normalization constant for the (B * n_{t|d}) term
		topicBetaMass += beta * n /	(tokensPerTopic[topic] + betaSum);	

		//	update the coefficients for the non-zero topics
		cachedCoefficients[topic] =	(alpha[topic] + n) / (tokensPerTopic[topic] + betaSum);
	}

	double topicTermMass = 0.0;

	std::vector<double> topicTermScores(numTopics);
	std::vector<int> topicTermIndices;
	std::vector<int> topicTermValues;
	int i;
	double score;

	//	Iterate over the positions (words) in the document 
	for (int position = 0; position < docLength; position++) {
		type = tokenSequence[position];
		oldTopic = oneDocTopics[position];

		if (oldTopic != UNASSIGNED_TOPIC) {
			//	Remove this token from all counts. 
				
			// Remove this topic's contribution to the 
			//  normalizing constants
			smoothingOnlyMass -= alpha[oldTopic] * beta / (tokensPerTopic[oldTopic] + betaSum);
			topicBetaMass -= beta * localTopicCounts[oldTopic] / (tokensPerTopic[oldTopic] + betaSum);
				
			// Decrement the local doc/topic counts
				
			localTopicCounts[oldTopic]--;
				
			// Maintain the dense index, if we are deleting
			//  the old topic
			if (localTopicCounts[oldTopic] == 0) {
					
				// First get to the dense location associated with
				//  the old topic.
					
				denseIndex = 0;
					
				// We know it's in there somewhere, so we don't 
				//  need bounds checking.
				while (localTopicIndex[denseIndex] != oldTopic) {
					denseIndex++;
				}
				
				// shift all remaining dense indices to the left.
				while (denseIndex < nonZeroTopics) {
					if (denseIndex < localTopicIndex.size()- 1) {
						localTopicIndex[denseIndex] = localTopicIndex[denseIndex + 1];
					}
					denseIndex++;
				}
					
				nonZeroTopics --;
			}

			// Decrement the global topic count totals
			tokensPerTopic[oldTopic]--;
			//assert(tokensPerTopic[oldTopic] >= 0) : "old Topic " + oldTopic + " below 0";
			

			// Add the old topic's contribution back into the
			//  normalizing constants.
			smoothingOnlyMass += alpha[oldTopic] * beta / (tokensPerTopic[oldTopic] + betaSum);
			topicBetaMass += beta * localTopicCounts[oldTopic] / (tokensPerTopic[oldTopic] + betaSum);

			// Reset the cached coefficient for this topic
			cachedCoefficients[oldTopic] = (alpha[oldTopic] + localTopicCounts[oldTopic]) / (tokensPerTopic[oldTopic] + betaSum);
		}


		// Now go over the type/topic counts, decrementing
		//  where appropriate, and calculating the score
		//  for each topic at the same time.

		int index = 0;
		int currentTopic, currentValue;
		bool alreadyDecremented = (oldTopic == UNASSIGNED_TOPIC);

		topicTermMass = 0.0;

		while (index < typeTopicCounts[type].size() && typeTopicCounts[type][index] > 0) {
			currentTopic = typeTopicCounts[type][index] & topicMask;
			currentValue = typeTopicCounts[type][index] >> topicBits;

			if (! alreadyDecremented && currentTopic == oldTopic) {

				// We're decrementing and adding up the 
				//  sampling weights at the same time, but
				//  decrementing may require us to reorder
				//  the topics, so after we're done here,
				//  look at this cell in the array again.

				currentValue --;
				if (currentValue == 0) {
					typeTopicCounts[type][index] = 0;
				}
				else {
					typeTopicCounts[type][index] = (currentValue << topicBits) + oldTopic;
				}
					
				// Shift the reduced value to the right, if necessary.

				int subIndex = index;
				while (subIndex < typeTopicCounts[type].size() - 1 && typeTopicCounts[type][subIndex] < typeTopicCounts[type][subIndex + 1]) {
					int temp = typeTopicCounts[type][subIndex];
					typeTopicCounts[type][subIndex] = typeTopicCounts[type][subIndex + 1];
					typeTopicCounts[type][subIndex + 1] = temp;
						
					subIndex++;
				}

				alreadyDecremented = true;
			}
			else {
				score = cachedCoefficients[currentTopic] * currentValue;
				topicTermMass += score;
				topicTermScores[index] = score;

				index++;
			}
		}
			
		boost::random::uniform_real_distribution<> unif(0,1);
		double u = unif(rng);
		while (u <= 0.0){
			u = unif(rng);
		}
		double sample = u * (smoothingOnlyMass + topicBetaMass + topicTermMass);
		double origSample = sample;

		//	Make sure it actually gets set
		newTopic = -1;

		if (sample < topicTermMass) {
			//topicTermCount++;

			i = -1;
			while (sample > 0) {
				i++;
				sample -= topicTermScores[i];
			}

			newTopic = typeTopicCounts[type][i] & topicMask;
			currentValue = typeTopicCounts[type][i] >> topicBits;
				
			typeTopicCounts[type][i] = ((currentValue + 1) << topicBits) + newTopic;

			// Bubble the new value up, if necessary
				
			while (i > 0 && typeTopicCounts[type][i] > typeTopicCounts[type][i - 1]) {
				int temp = typeTopicCounts[type][i];
				typeTopicCounts[type][i] = typeTopicCounts[type][i - 1];
				typeTopicCounts[type][i - 1] = temp;

				i--;
			}

		}
		else {
			sample -= topicTermMass;
			if (sample < topicBetaMass) {
				//betaTopicCount++;
				//std::cout << " one" << std::endl;
				sample /= beta;

				for (denseIndex = 0; denseIndex < nonZeroTopics; denseIndex++) {
					int topic = localTopicIndex[denseIndex];

					sample -= localTopicCounts[topic] / (tokensPerTopic[topic] + betaSum);

					if (sample <= 0.0) {
						newTopic = topic;
						break;
					}
				}

			}
			else {
				//std::cout << u << " " << sample << " " << smoothingOnlyMass << " " << topicBetaMass << " " << topicTermMass << std::endl;
				//std::cout << sample - topicBetaMass << " " << smoothingOnlyMass << std::endl;
				//smoothingOnlyCount++;
				/*std::cout << " two" << std::endl;
				double summ = 0;
				for (size_t t = 0; t < numTopics; t++) {
					summ += alpha[t] * beta / (tokensPerTopic[t] + betaSum);
					std::cout << alpha[t] << " " << tokensPerTopic[t] << "; ";
				}
				std::cout << " beta: " << beta << " betasum: "<< betaSum << std::endl;*/
				sample -= topicBetaMass;

				//std::cout << summ << " " << smoothingOnlyMass << " " << sample << std::endl;

				sample /= beta;

				newTopic = 0;
				sample -= alpha[newTopic] / (tokensPerTopic[newTopic] + betaSum);

				while (sample > 0.0) {
					newTopic++;
					sample -= alpha[newTopic] / (tokensPerTopic[newTopic] + betaSum);
				}
				
			}
			//std::cout << sample << " Topics " << newTopic << std::endl;
 
			// Move to the position for the new topic,
			//  which may be the first empty position if this
			//  is a new topic for this word.
				
			index = 0;
			while (typeTopicCounts[type][index] > 0 && (typeTopicCounts[type][index] & topicMask) != newTopic) {
				index++;
				if (index == typeTopicCounts[type].size()) {
					// TODO: ERROR
					// System.err.println("type: " + type + " new topic: " + newTopic);
					// for (int k=0; k<currentTypeTopicCounts.length; k++) {
					//	System.err.print((currentTypeTopicCounts[k] & topicMask) + ":" + (currentTypeTopicCounts[k] >> topicBits) + " ");
					//}
					//System.err.println();

				}
			}


			// index should now be set to the position of the new topic,
			//  which may be an empty cell at the end of the list.

			if (typeTopicCounts[type][index] == 0) {
				// inserting a new topic, guaranteed to be in
				//  order w.r.t. count, if not topic.
				typeTopicCounts[type][index] = (1 << topicBits) + newTopic;
			}
			else {
				currentValue = typeTopicCounts[type][index] >> topicBits;
				typeTopicCounts[type][index] = ((currentValue + 1) << topicBits) + newTopic;

				// Bubble the increased value left, if necessary
				while (index > 0 && typeTopicCounts[type][index] > typeTopicCounts[type][index - 1]) {
					int temp = typeTopicCounts[type][index];
					typeTopicCounts[type][index] = typeTopicCounts[type][index - 1];
					typeTopicCounts[type][index - 1] = temp;

					index--;
				}
			}

		}

		if (newTopic == -1) {
			//System.err.println("WorkerRunnable sampling error: "+ origSample + " " + sample + " " + smoothingOnlyMass + " " + 
			//		topicBetaMass + " " + topicTermMass);
			newTopic = numTopics-1; // TODO is this appropriate
			//throw new IllegalStateException ("WorkerRunnable: New topic not sampled.");
		}
		//assert(newTopic != -1);

		//			Put that new topic into the counts
		oneDocTopics[position] = newTopic;

		smoothingOnlyMass -= alpha[newTopic] * beta / (tokensPerTopic[newTopic] + betaSum);
		topicBetaMass -= beta * localTopicCounts[newTopic] / (tokensPerTopic[newTopic] + betaSum);

		localTopicCounts[newTopic]++;

		// If this is a new topic for this document,
		//  add the topic to the dense index.
		if (localTopicCounts[newTopic] == 1) {
				
			// First find the point where we 
			//  should insert the new topic by going to
			//  the end (which is the only reason we're keeping
			//  track of the number of non-zero
			//  topics) and working backwards

			denseIndex = nonZeroTopics;

			while (denseIndex > 0 && localTopicIndex[denseIndex - 1] > newTopic) {

				localTopicIndex[denseIndex] = localTopicIndex[denseIndex - 1];
				denseIndex--;
			}
				
			localTopicIndex[denseIndex] = newTopic;
			nonZeroTopics++;
		}

		tokensPerTopic[newTopic]++;

		//	update the coefficients for the non-zero topics
		cachedCoefficients[newTopic] = (alpha[newTopic] + localTopicCounts[newTopic]) / (tokensPerTopic[newTopic] + betaSum);

		smoothingOnlyMass += alpha[newTopic] * beta / (tokensPerTopic[newTopic] + betaSum);
		topicBetaMass += beta * localTopicCounts[newTopic] / (tokensPerTopic[newTopic] + betaSum);

	}

	if (shouldSaveState) {
		// Update the document-topic count histogram,
		//  for dirichlet estimation
		docLengthCounts[ docLength ]++;

		for (denseIndex = 0; denseIndex < nonZeroTopics; denseIndex++) {
			int topic = localTopicIndex[denseIndex];
				
			topicDocCounts[topic][ localTopicCounts[topic] ]++;
		}
	}

	//	Clean up our mess: reset the coefficients to values with only
	//	smoothing. The next doc will update its own non-zero topics...

	for (denseIndex = 0; denseIndex < nonZeroTopics; denseIndex++) {
		int topic = localTopicIndex[denseIndex];

		cachedCoefficients[topic] = alpha[topic] / (tokensPerTopic[topic] + betaSum);
	}
	
	//std::cout << " HERE2 " <<  std::endl;
}

void WorkerRunnable::buildLocalTypeTopicCounts() 
{

	// Clear the topic totals
	for (size_t ind = 0; ind < tokensPerTopic.size(); ++ ind){
		tokensPerTopic[ind] = 0;
	}

	// Clear the type/topic counts, only 
	//  looking at the entries before the first 0 entry.

	for (int type = 0; type < typeTopicCounts.size(); type++) {

		std::vector<int>& topicCounts = typeTopicCounts[type];
			
		int position = 0;
		while (position < topicCounts.size() && topicCounts[position] > 0) {
			topicCounts[position] = 0;
			position++;
		}
	}

	for (int doc = startDoc; doc < texts.size() && doc < startDoc + numDocs; doc++) {


		std::vector<int>& tokens = texts[doc]->getFeatures();
		std::vector<int>& topics = texts[doc]->getFeatureTopic();

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
					//System.out.println("overflow on type " + type);
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
				while (index > 0 &&	currentTypeTopicCounts[index] > currentTypeTopicCounts[index - 1]) {
					int temp = currentTypeTopicCounts[index];
					currentTypeTopicCounts[index] = currentTypeTopicCounts[index - 1];
					currentTypeTopicCounts[index - 1] = temp;
						
					index--;
				}
			}
		}
	}
}

}
}


