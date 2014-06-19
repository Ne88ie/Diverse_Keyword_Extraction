/*
 * File:   utils.h
 * Author: annie
 *
 * Created on 8 Июнь 2014 г., 3:18
 */

#ifndef UTILS_H
#define	UTILS_H

#include <string>
#include <fstream>
#include <codecvt>
#include <cwctype>
#include <unordered_set>

namespace mas
{
namespace utils
{
    /* convert str->wstr */
    std::wstring s2ws(const std::string& str);

    /* convert wstr-wstr*/
    std::string ws2s(const std::wstring& wstr);

    /* normalize word, tolower and erase punctual */
    void toStandard(std::string &str);

    void delPunctuation(std::string &str);

    std::unordered_set<std::string> getStopWords();

    /* check wether word not in stop-words */
    bool goodWord(std::string const & word);

} // namespace utils
} // namespace mas



#endif	/* UTILS_H */

