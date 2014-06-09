#include <cstdlib>
#include <vector>
#include <string>
#include <utility>

namespace stc{
namespace textMining{

class Text
{
public:
	Text(std::string& name, std::vector<int>& encodedFeatures);
	~Text();

	std::vector<int>& getFeatures() {return features;}

	std::vector<int>& getFeatureTopic() {return featureTopic;}

	std::string getName() { return name;}

        void setTopicDistribution(const std::vector<std::pair<int, double>> & topicDistribution)
        {
            this->topicDistribution = topicDistribution;
        }

        const std::vector<std::pair<int, double>>& getTopicDistribution() const
        {
            return topicDistribution;
        }

private:
	std::vector<int> features;
	std::vector<int> featureTopic;
	std::vector<std::pair<int, double>> topicDistribution;
	std::string name;
};

}
}