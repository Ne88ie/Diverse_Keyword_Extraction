// -*- C++ -*-
/*
 * File:   DiverseKeyword.h
 * Author: annie
 *
 * Created on 9 Июнь 2014 г., 1:39
 */

#ifndef DIVERSE_KEYWORD_H
#define	DIVERSE_KEYWORD_H

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <utility>
#include <cmath>

#include "TopicModel.h"

using namespace stc::textMining;

namespace mas
{

class DiverseKeyword
{
    size_t numKeyword;
    double lambda;
    size_t numTopics;
    std::vector<size_t> setKeywords;
    std::vector<std::vector<size_t>> allKeywords;
    std::vector<std::vector<double>> topicDistributionOnWords;

    const Text * tempText;
    double maxComputedR;
    std::vector<double> bufferComputedR;

    const TopicModel & topicModel;
    const std::vector<Text *> & texts;

    class reward_function;

    friend class reward_function;

//    class reward_function : public std::unary_function< int, double >
//    {
//        Diverse_Keyword * nested;
//    public:
//        reward_function(Diverse_Keyword * th);
//        double operator() (const int indWord) const;
//    };


    void fillTopicDistributionOnWords();

    void fillSetKeywords(const Text * text);

    void fillTextKeywords();

public:

    DiverseKeyword(const TopicModel & topModel, size_t nKeyword = 2, double lmbda = 0.75);

    const std::vector<std::vector<size_t>> & getAllKeywords() const { return allKeywords; }

    std::vector<std::vector<std::string>> getAllDecodedKeywords();

    void printKeywords(std::ostream & out) const;
};

} // namespace mas



#endif	/* DIVERSE_KEYWORD_H */

