#include "handle.h"

using namespace stc::textMining;
using namespace mas::utils;


typedef std::vector<std::string> vs;
typedef std::vector<std::vector<std::string>> vvs;

bool TO_LOWERCASE = false;
bool DEL_STOP_WORDS = false;
bool CHANGE = false;
std::string SOURCETEXTS;
std::pair<vs, vvs> TEXT;

std::pair<vs, vvs> getContent(bool  to_lowercase, bool del_stop_words, std::string fileName)
{
    std::ifstream fin(fileName);

    vs names;
    vvs contents;
    std::string item, pitem = "";

    if (del_stop_words)  to_lowercase = true;
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
            if ( to_lowercase)
            {
                toStandard(pitem);
            }
            else
            {
                delPunctuation(pitem);
            }
            if (!pitem.empty() && (!del_stop_words || (del_stop_words && goodWord(pitem))))
            {
                contents[contents.size() - 1].push_back(pitem);
            }
        }
        pitem = item;
    }

    fin.close();

    std::cout << "... DATA LOADED\n";

    return std::make_pair(names, contents);
}


namespace mas
{
    void handleText(std::string sourceTexts,
                    size_t numTopics,
                    size_t topWords,
                    size_t numKeywords,
                    bool to_lowercase,
                    bool del_stop_words,
                    std::string fileTopWords,
                    std::string fileDocTopics,
                    std::string fileKeywords)
    {
        std::cout << "HANDLE " << sourceTexts<< " STARTED ...\n";

        if (sourceTexts.empty())
        {
            std::cout << "ERROR: SPECIFY SOURCE FILE!\n";
            return;
        }

        if (!CHANGE || (to_lowercase != TO_LOWERCASE || del_stop_words != DEL_STOP_WORDS) || SOURCETEXTS != sourceTexts)
        {
            TEXT = getContent(to_lowercase, del_stop_words, sourceTexts);
            CHANGE = true;
            SOURCETEXTS = sourceTexts;
            TO_LOWERCASE = to_lowercase;
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
        for (size_t ind = 0; ind < std::min(10, int(contents.size())); ++ind)
        {
            std::cout << names[ind] << std::endl;
            for (size_t i = 0; i < std::min(5, int(contents[ind].size())); ++i)
            {
                setlocale(LC_ALL, "Russian");
                std::cout << contents[ind][i] << " ";
            }
            std::cout << std::endl;
        } */

        topicModel.addDocuments(names, contents);

        int showTopicsInterval = 50, numIterations = 1000, optimizeInterval = 0, optimizeBurnIn = 200;

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


        std::string log_lowercase = to_lowercase ? "_LOWERCASE" : "";
        std::string log_stopwords = del_stop_words ? "_DELL_STOP-WORDS" : "";
        std::string log_info = "_ON_" + std::to_string(numTopics) + "_TOPICS" + log_lowercase + log_stopwords;


        if (fileTopWords.empty())
        {
            std::cout << "\n\n--------------*** OUT TOP WORDS FOR TOPICS ***--------------\n";
            topicModel.printTopWords(std::cout, topWords, true); // "false" for not usingNewLines
            std::cout << "\n";
        }
        else
        {
            std::ofstream outTopWords(fileTopWords + log_info + ".txt");
            topicModel.printTopWords(outTopWords, topWords, true); // "false" for not usingNewLines
            outTopWords.close();
        }


        double threshold = 0;
        int maxTopics = -1; // all

        if (fileDocTopics.empty())
        {
            std::cout << "\n\n--------------*** OUT DOCUMENT TOPICS DISTRIBUTION ***--------------\n";
            topicModel.printDocumentTopics(std::cout, threshold, maxTopics);
            std::cout << "\n";
        }
        else
        {
            std::ofstream outDocumentTopics(fileDocTopics + log_info + ".txt");
            topicModel.printDocumentTopics(outDocumentTopics, threshold, maxTopics);
            outDocumentTopics.close();
        }


        DiverseKeyword diverseKeyword(topicModel, numKeywords);
        if (fileKeywords.empty())
        {
            std::cout << "\n\n--------------*** OUT DOCUMENT DIVERSE KEYWORDS ***--------------\n";
            diverseKeyword.printKeywords(std::cout);
            std::cout << "\n";
        }
        else
        {
            std::ofstream outKeywords(fileKeywords + log_info + ".txt");
            diverseKeyword.printKeywords(outKeywords);
            outKeywords.close();
        }

        std::transform(log_info.begin(), log_info.end(), log_info.begin(), ::tolower);
        std::replace(log_info.begin(), log_info.end(), '_', ' ');
        boost::trim(log_info);
        std::cout << "... HANDLE FINISHED (" << log_info << ") \n\n";
    }
} // namespace mas
