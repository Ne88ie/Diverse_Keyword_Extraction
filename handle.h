/*
 * File:   modeled_topics.h
 * Author: annie
 *
 * Created on 8 Июнь 2014 г., 19:38
 */

#ifndef MODELED_TOPICS_H
#define	MODELED_TOPICS_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>

#include "TopicModel.h"
#include "DiverseKeyword.h"
#include "utils.h"

namespace mas
{
    void handleText(size_t numTopics = 2, size_t numKeywords = 2, size_t topWords = 20, bool normalise = false, bool del_stop_words = false,
            std::string sourceTexts = "", std::string fileTopWords = "",  std::string fileDocTopics = "", std::string fileKeywords = "");
} // namespace mas


#endif	/* MODELED_TOPICS_H */

