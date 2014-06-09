#include <cstdlib>
#include <vector>
#include <string>

namespace stc{

namespace textMining{
class Text
{
public:
	Text(std::string& name, std::vector<int>& encodedFeatures);
	~Text();

	std::vector<int>& getFeatures() {return features;}

	std::vector<int>& getFeatureTopic() {return featureTopic;}

	std::string getName(){ return name;}

private:

	std::vector<int> features;
	std::vector<int> featureTopic;
	std::vector<double> topicDistribution; // not used
	std::string name;

};
}
}