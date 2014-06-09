#include "diverse_keyword_extract.h"

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
}
}


//std::vector<std::string> SETWORDS;
//double computedR = 0;
//std::vector<size_t> & TOPICS;

//double computeR(const std::vector<std::string> & set)
//{
//    double sum = 0;
//    for (size_t i : TOPICS)
//    {
//        sum += 1; //p(z|w) p(z|t)
//
//    }
//}


class reward_function : public std::unary_function< std::string, double >
{
public:

    double operator() (const std::string & word) const
    {
        return pow(4, 6);
    }
};


namespace mas
{
    using namespace stc::textMining;

//    void extract(size_t k = 5, const TopicModel & topicModel) //const std::vector<std::string> & text, const std::vector<size_t> & topics)
//    {
//        TOPICS = topics;
//        std::vector<std::string> bufferText(text);
//        while (SETWORDS.size() < k)
//        {
//            std::vector<std::string>::iterator newWord(mas::util::argmax(bufferText.begin(), bufferText.end(), reward_function()));
//            SETWORDS.push_back(*newWord);
//            std::swap(newWord, SETWORDS.back());
//            bufferText.pop_back();
//        }
//    }
}
