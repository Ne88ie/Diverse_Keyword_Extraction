#include <cstdlib>
#include <vector>

class Topic
{
public:
	Topic();
	~Topic();

private:

	std::vector<double> featureDistribution;

};
