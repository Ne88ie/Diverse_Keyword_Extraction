// TopicModeling.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>

#include "TopicModel.h"
#include "utils.h"

using namespace stc::textMining;
using namespace mas::utils;


int main(int argc, char* argv[])
{
    double alpha = 50.0, beta = 0.01;
    size_t numTopics = 5; // изначально было 2
    TopicModel topicModel(numTopics, alpha, beta);
    int randomSeed = 0;
    if (randomSeed != 0)
    {
        topicModel.setRandomSeed(randomSeed);
    }

//    std::string fileName = "C:/mallet-2.0.7/h_layer_mallet.txt";
    std::string fileName = "/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/20000_mallet_small.txt"; // отладка
    std::ifstream fin(fileName);

    std::vector<std::string> names;
    std::vector<std::vector < std::string>> contents;
    std::string item, pitem = "";

    while (!fin.eof()) {
        fin >> item;
        if (item[0] == 'X' && item.length() == 1) {
            names.push_back(pitem);
            contents.push_back(std::vector<std::string>(0));
        } else if (pitem.length() > 3 && (pitem[0] != 'X' || pitem.length() > 1)) {
            toStandard(pitem);
            if (goodWord(pitem)) {
                contents[contents.size() - 1].push_back(pitem);
            }
        }
        pitem = item;
    }
    /*
    for (size_t ind = 0; ind < min(10, contents.size()); ++ind){
            std::cout << names[ind] << std::endl;
            for (size_t i = 0; i < min(5,contents[ind].size()); ++ i){
                    setlocale(LC_ALL, "Russian");
                    std::cout << contents[ind][i] << " ";
            }
            std::cout << std::endl;
    }*/
    topicModel.addDocuments(names, contents);
    std::cout << "DATA LOADED" << std::endl;
    int showTopicsInterval = 50, topWords = 20, numIterations = 1000, optimizeInterval = 0, optimizeBurnIn = 200;
    topicModel.setTopicDisplay(showTopicsInterval, topWords);

    topicModel.setNumIterations(numIterations);
    topicModel.setOptimizeInterval(optimizeInterval);
    topicModel.setBurninPeriod(optimizeBurnIn);
    bool useSymmetricAlpha = false;
    topicModel.setSymmetricAlpha(useSymmetricAlpha);

    int outputStateInterval = 0, outputModelInterval = 0;
    std::string stateFile, outputModelFilename;
    if (outputStateInterval != 0)
    {
        topicModel.setSaveState(outputStateInterval, stateFile);
    }

    if (outputModelInterval != 0)
    {
        topicModel.setSaveSerializedModel(outputModelInterval, outputModelFilename);
    }

    int numThreads = 1;
    topicModel.setNumThreads(numThreads);
    topicModel.setRandomSeed(13232);
    topicModel.estimate();

    std::ofstream outTopWords("/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/top_words_small_on5topics_normalize.txt");
    outTopWords << topicModel.displayTopWords(topWords, true);
//    outTopWords << topicModel.displayTopWords(topWords, false);
    outTopWords.close();

    std::ofstream outt("/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/doc-topics_small_on5topics_normalize.txt");
    topicModel.displayDocumentTopics(outt, 0, -1);
    outt.close();

    //std::cout << "Press any key to out" << std::endl;
    //std::getchar();
    return 0;
}
