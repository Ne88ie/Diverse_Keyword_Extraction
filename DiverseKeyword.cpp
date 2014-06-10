#include "DiverseKeyword.h"

namespace mas
{
    namespace util
    {
        template<typename IteratorT, typename HeuristicFunctorT>
        IteratorT argmax(IteratorT && it, const IteratorT & end, const HeuristicFunctorT & functor)
        {
            IteratorT best(it++);
            typename HeuristicFunctorT::result_type best_value(functor(*best));

            for (; it != end; ++it)
            {
                typename HeuristicFunctorT::result_type value(functor(*it));

                if (value > best_value)
                {
                    best_value = value;
                    best = it;
                }
            }

            return best;
        }

        template<typename IteratorT, typename HeuristicFunctorT>
        inline IteratorT argmax(const IteratorT & begin, const IteratorT & end, const HeuristicFunctorT & functor)
        {
            return argmax(IteratorT(begin), end, functor);
        }
    } // namespace util


    using namespace mas::util;


    class DiverseKeyword::reward_function : public std::unary_function< int, double >
    {
        DiverseKeyword * nested;
    public:
        reward_function(DiverseKeyword * th)
            : nested(th)
        {
        }

        double operator() (const int indWord) const
        {
            double res = 0;
            const std::vector<std::pair<int, double>>& topicDistributionInDoc = nested->tempText->getTopicDistribution();

            for (size_t indTopic = 0; indTopic < nested->numTopics; ++indTopic)
            {
                double computedR = nested->topicDistributionOnWords.at(indWord).at(indTopic) * topicDistributionInDoc.at(indTopic).second;
                res += topicDistributionInDoc.at(indTopic).second * pow(computedR + nested->bufferComputedR.at(indTopic), nested->lambda);
            }

            nested->maxComputedR = std::max(res, nested->maxComputedR);

            return res;
        }
    };


    void DiverseKeyword::fillTopicDistributionOnWords()
    {
        const std::vector<int> & typeTotals = topicModel.getTypeTotals();
        const std::vector<std::vector<std::pair<int, double>>> & topicSortedWords = topicModel.getTopicSortedWords();
        const std::vector<int> & countTokensPerTopic = topicModel.getCountTokensPerTopic();

        for (size_t i = 0; i < numTopics; ++i)
        {
            for (auto wordFreq : topicSortedWords.at(i))
            {
                topicDistributionOnWords.at(wordFreq.first).at(i) =
                        wordFreq.second / countTokensPerTopic.at(i) * typeTotals.at(i);
            }
        }

        for (auto topicDistributon : topicDistributionOnWords)
        {
            double denominator = 0;
            for (auto topic : topicDistributon)
            {
                denominator += topic;
            }

            for (auto topic : topicDistributon)
            {
                topic /= denominator;
            }
        }
    }


    std::vector<int> DiverseKeyword::getSetKeywords(const Text * text)
    {
        std::vector<int> setKeywords;
        const std::vector<int> & allWords = text->getFeatures();
        std::unordered_set<int> uniqueWords(allWords.begin(), allWords.end());

        tempText = text;

        while (setKeywords.size() < std::min(numKeyword, uniqueWords.size()))
        {
            maxComputedR = 0;
            std::unordered_set<int>::iterator nextWord(mas::util::argmax(uniqueWords.begin(), uniqueWords.end(), reward_function(this)));
            setKeywords.push_back(*nextWord);
            uniqueWords.erase(nextWord);
            for (auto ComputedR : bufferComputedR)
            {
                ComputedR += maxComputedR;
            }
        }

        return setKeywords;
    }


    void DiverseKeyword::fillTextKeywords()
    {
        fillTopicDistributionOnWords();

        for (auto text: texts)
        {
            allKeywords.push_back(getSetKeywords(text));
        }
    }


    DiverseKeyword::DiverseKeyword(const TopicModel & topModel, size_t nKeyword, double lmbda)
            : numKeyword(nKeyword)
            , lambda(lmbda)
            , numTopics(topModel.getNumTopics())
            , topicModel(topModel)
            , texts(topModel.getTexts())
    {
        allKeywords.reserve(texts.size());
        bufferComputedR.assign(numTopics, 0);
        topicDistributionOnWords.assign(topicModel.getDictionary().getFeaturesNum(), std::vector<double>(numTopics, 0));
        fillTextKeywords();
    }


    std::vector<std::vector<std::string>> DiverseKeyword::getAllDecodedKeywords()
    {
        std::vector<std::vector<std::string>> AllKeywordsDecoded;
        try
        {
            AllKeywordsDecoded.assign(allKeywords.size(), std::vector<std::string>(numKeyword, ""));

            for (size_t indText = 0; indText < allKeywords.size(); ++indText)
                {
                    for (size_t i = 0; i < numKeyword; ++i)
                    {
                        int indWord = allKeywords.at(indText).at(i);
                        std::string word = topicModel.getDictionary().getFeatureDecoded(indWord);
                        AllKeywordsDecoded.at(indText).at(i) = word;
                    }
                }
        }
        catch(...)
        {
            std::cerr << "Can't return keywords\n";
        }

        return AllKeywordsDecoded;
    }


    void DiverseKeyword::printKeywords(std::ostream & out) const
    {
        out << "#doc name keywords ...\n";
        try
        {
            for (size_t i = 0; i < allKeywords.size(); ++i)
            {
                out << i + 1 << "\t" << texts.at(i)->getName();

                for (auto ind: allKeywords.at(i))
                {
                    out << "\t" << topicModel.getDictionary().getFeatureDecoded(ind);
                }

                out << "\n";
            }
        }
        catch(...)
        {
            std::cerr << "Can't print keywords\n";
        }
    }

} // namespace mas
