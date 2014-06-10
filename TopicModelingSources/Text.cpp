//#include "stdafx.h"

#include "Text.h"

namespace stc
{
namespace textMining
{

    Text::Text(std::string& name, std::vector<int>& encodedFeatures)
    {
        this->name = name;
        features.insert(features.end(), encodedFeatures.begin(), encodedFeatures.end());
        featureTopic.resize(encodedFeatures.size());
    }

    Text::~Text()
    {
        name.clear();
        features.clear();
    }

} // namespace textMining
} // namespace stc