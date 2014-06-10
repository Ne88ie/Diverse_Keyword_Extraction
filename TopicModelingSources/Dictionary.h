#ifndef DICTIONARY_H
#define	DICTIONARY_H

#include <cstdlib>
#include <vector>
#include <string>
#include <map>

namespace stc
{
namespace textMining
{

class Dictionary
{
public:
    Dictionary();
    ~Dictionary();

    void addFeatures(std::vector<std::string> & features, std::vector<int> & encodedFeatures);

    int getFeatureEncoded(std::string feature);

    std::string getFeatureDecoded(int featureCode) const;

    void stopGrowth();

    int getFeaturesNum() const {return featuresNum; }

    std::vector<int> getFeatureTotals() const {return featureTotals;}

private:

    void addNewFeature(std::string& feature);

    std::map<std::string, int> featureEncoding;
    std::vector<std::string> featureDecoding;
    std::vector<int> featureTotals;

    bool growthStopped;
    int featuresNum;

};

} // namespace textMining
} // namespace stc

#endif	/* DICTIONARY_H */