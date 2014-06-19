#include "handle.h"
#include "convert.h"

using mas::handleText;
using mas::convert;

void globalOUT(const std::string & base)
{
    std::string prefixToResult = "data/results/";

    std::string pathBase    = "data/" + base;
    std::string sourceTexts = prefixToResult + base + "_convert_out.txt";
    size_t numTopics        = convert(pathBase, sourceTexts);
    size_t numTopWords      = 20;
    size_t numKeywords      = 6;


    std::string outTopWords    = prefixToResult + base + "_top_words";
    std::string outDocTopics   = prefixToResult + base + "_doc-topics";
    std::string outDocKeywords = prefixToResult + base + "_doc-keywords";

    handleText(sourceTexts, numTopics, numTopWords, numKeywords, false, false, outTopWords, outDocTopics, outDocKeywords);
    handleText(sourceTexts, numTopics, numTopWords, numKeywords, true, true, outTopWords, outDocTopics, outDocKeywords);
}

int main(int argc, char* argv[])
{
    // convert and handle first base - Base_RGD
    globalOUT("Base_RGD");

    // convert and handle second base - Base_HotLine
    globalOUT("Base_HotLine");

    // convert and handle third base - Base_News_small
    globalOUT("Base_News_small");



// // examples of work with TopicModeling and DiverseKeywordExtraction without convert
//    std::string sourceTexts     = "data/example_small.txt";
//    std::string outTopWords     = "data/results/small_top_words"; // std::out by default
//    std::string outDocTopics    = "data/results/small_doc-topics"; // std::out by default
//    std::string outDocKeywords  = "data/results/small_doc-keywords"; // std::out by default
//
//    size_t numTopics    = 5;     // 2 by default
//    size_t numTopWords  = 20;    // 5 by default
//    size_t numKeywords  = 6;     // 3 by default
//    bool is_normalize   = false; // false by default
//    bool del_stop_words = false; // false by default
//
//    handleText(sourceTexts, numTopics, numTopWords, numKeywords, is_normalize, del_stop_words, outTopWords, outDocTopics, outDocKeywords);
//    handleText(sourceTexts);
//    handleText(sourceTexts, numTopics, numTopWords, numKeywords, true, true, outTopWords, outDocTopics, outDocKeywords);
//    handleText(sourceTexts, numTopics, numTopWords, numKeywords, true, true);

    return 0;
}
