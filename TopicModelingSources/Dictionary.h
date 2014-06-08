#include <cstdlib>
#include <vector>
#include <string>
#include <map>

namespace stc{

namespace textMining{
class Dictionary
{
public:
	Dictionary();
	~Dictionary();

	void addFeatures(std::vector<std::string> & features, std::vector<int> & encodedFeatures);

	int getFeatureEncoded(std::string feature);

	std::string getFeatureDecoded(int featureCode);

	void stopGrowth();
	
	int getFeaturesNum() {return featuresNum; }

	std::vector<int> getFeatureTotals(){return featureTotals;}

private:

	void addNewFeature(std::string& feature);

	std::map<std::string,int> featureEncoding;
	std::vector<std::string> featureDecoding;
	std::vector<int> featureTotals;

	bool growthStopped;
	int featuresNum;

};
}
}