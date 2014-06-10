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


//        double computeR(const std::vector<size_t> & set, size_t topic)
//        {
//            const std::vector<std::pair<int, double>>& topicDistributionInDoc = tempText->getTopicDistribution();
//
//            double sum = 0;
//            for (size_t i : set)
//            {
//                sum += topicDistributionOnWords.at(i).at(topic) * topicDistributionInDoc.at(topic).second;
//            }
//
//            return sum;
//        }

//        double computeR(size_t indWord, size_t topic, const std::vector<std::pair<int, double>>& topicDistributionInDoc)
//        {
//            double res = topicDistributionOnWords.at(i).at(topic) * topicDistributionInDoc.at(topic).second;
//
//            return ;
//        }


//        friend class reward_function;

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
            nested->maxComputedR = 0;

            const std::vector<std::pair<int, double>>& topicDistributionInDoc = nested->tempText->getTopicDistribution();

            for (size_t i = 0; i < nested->numTopics; ++i)
            {
//                res += topicDistributionInDoc.at(i).second *
//                        pow(nested->computeR(std::vector<int>(1, indWord), i) + nested->bufferComputedR.at(i), nested->lambda);
                double computedR = nested->topicDistributionOnWords.at(indWord).at(i) * topicDistributionInDoc.at(i).second;
                nested->maxComputedR = std::max(computedR, nested->maxComputedR);
                res += topicDistributionInDoc.at(i).second * pow(computedR + nested->bufferComputedR.at(i), nested->lambda);
            }

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


    void DiverseKeyword::fillSetKeywords(const Text * text)
    {
        setKeywords.clear();
        std::vector<int> words = text->getFeatures();
        tempText = text;

        while (setKeywords.size() < numKeyword)
        {
            std::vector<int>::iterator nextWord(mas::util::argmax(words.begin(), words.end(), reward_function(this)));
            setKeywords.push_back(*nextWord);
            std::swap(*nextWord, words.back());
            words.pop_back();
            for (auto ComputedR : bufferComputedR)
            {
                ComputedR += maxComputedR;
            }
        }
    }


    void DiverseKeyword::fillTextKeywords()
    {
        fillTopicDistributionOnWords();

        for (auto text: texts)
        {
            fillSetKeywords(text);
            allKeywords.push_back(setKeywords);
        }
    }


    DiverseKeyword::DiverseKeyword(const TopicModel & topModel, size_t nKeyword, double lmbda)
            : numKeyword(nKeyword)
            , lambda(lmbda)
            , numTopics(topModel.getNumTopics())
            , topicModel(topModel)
            , texts(topModel.getTexts())
    {
        allKeywords.resize(texts.size());
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
                        size_t indWord = allKeywords.at(indText).at(i);
                        std::string word = topicModel.getDictionary().getFeatureDecoded(indWord);
                        AllKeywordsDecoded.at(indText).at(i) = word;
                    }
                }
        }
        catch(...)
        {
            std::cerr << "Can't return top keywords\n";
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
            std::cerr << "Can't print tkeywords\n";
        }
    }

} // namespace mas
