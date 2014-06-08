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

namespace mas {
namespace utils {

    // convert wstr->str
    std::wstring s2ws(const std::string& str) {
        typedef std::codecvt_utf8<wchar_t> convert_typeX;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.from_bytes(str);
    }

    // convert str->wstr
    std::string ws2s(const std::wstring& wstr) {
        typedef std::codecvt_utf8<wchar_t> convert_typeX;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.to_bytes(wstr);
    }

    // normalize word, tolower and erase punctual
    void toStandard (std::string &str) {
        std::wstring ABC = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::wstring abc = L"абвгдеёжзийклмнопрстуфхцчшщъыьэюяabcdefghijklmnopqrstuvwxyz";

        std::wstring wstr = s2ws(str);
        for (size_t i = 0; i < wstr.size() ; ++i){
            if (!iswalnum(wstr[i]) && wstr[i] != '-' && (ABC + abc).find(wstr[i]) == std::string::npos){
                wstr.erase(wstr.begin() + i);
                --i;
            } else {
                size_t found = ABC.find(wstr[i]);
                if (found != std::string::npos){
                    wstr[i] = abc[found];
                }
            }
        }
        str = ws2s(wstr);
    }


    std::unordered_set<std::string> getStopWords(){
        std::unordered_set<std::string> set;
        std::string fileName = "/Users/annie/NetBeansProjects/Diverse_Keyword_Extraction/data/ru_stopwords_from_voyant.txt"; // отладка
        std::ifstream fin(fileName);
        std::string word;
        while (!fin.eof()){
            fin >> word;
            set.insert(word);
        }
        fin.close();
        return set;
    }


    // check wether word not in stop-words
    bool goodWord(std::string const & word) {
        static std::unordered_set<std::string> stopWords = getStopWords();
        if (stopWords.count(word) > 0) {
            return false;
        }
        return true;
    }

}
}



#endif	/* UTILS_H */

