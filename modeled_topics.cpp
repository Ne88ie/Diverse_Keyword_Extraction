#include "modeled_topics.h"

using namespace stc::textMining;
using namespace mas::utils;


typedef std::vector<std::string> vs;
typedef std::vector<std::vector<std::string>> vvs;

bool IS_NORMALIZE = false;
bool DEL_STOP_WORDS = false;
bool CHANGE = false;
std::pair<vs, vvs> TEXT;

std::pair<vs, vvs> get_content(bool is_normalize, bool del_stop_words)
{
    std::string fileName = "/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/20000_mallet_small.txt";
    std::ifstream fin(fileName);

    vs names;
    vvs contents;
    std::string item, pitem = "";

    if (del_stop_words) is_normalize = true;
    while (!fin.eof())
    {
        fin >> item;
        if (item[0] == 'X' && item.length() == 1)
        {
            names.push_back(pitem);
            contents.push_back(std::vector<std::string>(0));
        }
        else if (pitem.length() > 3 && (pitem[0] != 'X' || pitem.length() > 1))
        {
            if (is_normalize)
            {
                toStandard(pitem);
            }
            if (!del_stop_words || (del_stop_words && goodWord(pitem)))
            {
                contents[contents.size() - 1].push_back(pitem);
            }
        }
        pitem = item;
    }

    fin.close();

    std::cout << "DATA LOADED" << std::endl;

    return std::make_pair(names, contents);
}


namespace mas
{
    void modeled_topics(size_t numTopics,     bool is_normalize,      bool del_stop_words)
    { /* by deault      size_t numTopics = 2, bool normalise = false, bool del_stop_words = false */

        if (!CHANGE || (is_normalize != IS_NORMALIZE || del_stop_words != DEL_STOP_WORDS))
        {
            TEXT = get_content(is_normalize, del_stop_words);
            CHANGE = true;
            IS_NORMALIZE = is_normalize;
            DEL_STOP_WORDS = del_stop_words;
        }

        vs & names = TEXT.first;
        vvs & contents = TEXT.second;

        double alpha = 50.0, beta = 0.01;
        TopicModel topicModel(numTopics, alpha, beta);
        int randomSeed = 0;
        if (randomSeed != 0)
        {
            topicModel.setRandomSeed(randomSeed);
        }

        /*
        for (size_t ind = 0; ind < std::min(10, int(contents.size())); ++ind){
                std::cout << names[ind] << std::endl;
                for (size_t i = 0; i < std::min(5, int(contents[ind].size())); ++ i){
                        setlocale(LC_ALL, "Russian");
                        std::cout << contents[ind][i] << " ";
                }
                std::cout << std::endl;
        } */

        topicModel.addDocuments(names, contents);
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

        std::string log_normalize = is_normalize ? "_normalize" : "";
        std::ofstream outTopWords("/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/top_words_small_on"
                                    + std::to_string(numTopics) + "topics"+ log_normalize + ".txt");
        outTopWords << topicModel.displayTopWords(topWords, true); // "false" for not usingNewLines
        outTopWords.close();

        std::ofstream outDocumentTopics("/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/doc-topics_small_on"
                            + std::to_string(numTopics) + "topics"+ log_normalize + ".txt");
        topicModel.displayDocumentTopics(outDocumentTopics, 0, -1);
        outDocumentTopics.close();

        std::cout << "PROCESSING TOPIC MODEL(" << numTopics << log_normalize << ") IS COMPLETED OVER\n";
    }
}
