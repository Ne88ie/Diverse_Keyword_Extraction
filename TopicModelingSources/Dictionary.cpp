#include "Dictionary.h"

namespace stc
{
namespace textMining
{

    Dictionary::Dictionary()
    {
        featuresNum = 0;
        growthStopped = false;
    }

    Dictionary::~Dictionary()
    {
        featureEncoding.clear();
        featureDecoding.clear();
        featureTotals.clear();
    }

    void Dictionary::addNewFeature(std::string& feature)
    {
        featureEncoding[feature] = featuresNum;
        featureDecoding.push_back(feature);
        featureTotals.push_back(1);
        ++featuresNum;
    }

    void Dictionary::addFeatures(std::vector<std::string> & features, std::vector<int> & encodedFeatures)
    {
        for (size_t ind = 0; ind < features.size(); ++ind)
        {
            std::map<std::string, int>::iterator itMap = featureEncoding.find(features[ind]);
            if (itMap == featureEncoding.end())
            {
                addNewFeature(features[ind]);
            }
            else
            {
                ++featureTotals[itMap->second];
            }
            encodedFeatures[ind] = featureEncoding[features[ind]];
        }
    }

    int Dictionary::getFeatureEncoded(std::string feature)
    {
        return featureEncoding[feature];
    }

    std::string Dictionary::getFeatureDecoded(int featureCode) const
    {
        return featureDecoding[featureCode];
    }

    void Dictionary::stopGrowth()
    {
        growthStopped = true;
    }

} // namespace textMining
} // namespace stc