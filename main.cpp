#include "handle.h"

using mas::handleText;

int main(int argc, char* argv[])
{

    std::string sourceTexts     = "data/20000_mallet_small.txt";
    std::string outTopWords     = "data/top_words_small";
    std::string outDocTopics    = "data/doc-topics_small";
    std::string outDocKeywords  = "data/doc-keywords_small";

    size_t numTopics = 5;
    size_t numKeywords = 6;
    size_t numTopWords = 20;

    handleText(numTopics, numKeywords, numTopWords, false, false, sourceTexts, outTopWords, outDocTopics, outDocKeywords);
    handleText(numTopics, numKeywords, numTopWords, true, true, sourceTexts, outTopWords, outDocTopics, outDocKeywords);
    handleText(numTopics, numKeywords, numTopWords, true, true, sourceTexts);

    return 0;
}
