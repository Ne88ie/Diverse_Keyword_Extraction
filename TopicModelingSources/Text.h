#ifndef TEXT_H
#define	TEXT_H

#include <cstdlib>
#include <vector>
#include <string>
#include <utility>

namespace stc
{
namespace textMining
{

    class Text
    {
    public:
        Text(std::string& name, std::vector<int>& encodedFeatures);
        ~Text();

        std::vector<int>& getFeatures()
        {
            return features;
        }

        const std::vector<int>& getFeatures() const
        {
            return features;
        }

        std::vector<int>& getFeatureTopic()
        {
            return featureTopic;
        }

        const std::vector<int>& getFeatureTopic() const
        {
            return featureTopic;
        }

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

} // namespace textMining
} // namespace stc

#endif	/* TEXT_H */