#include "utils.h"

namespace mas
{
namespace utils
{

    std::wstring s2ws(const std::string& str)
    {
        typedef std::codecvt_utf8<wchar_t> convert_typeX;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.from_bytes(str);
    }

    std::string ws2s(const std::wstring& wstr)
    {
        typedef std::codecvt_utf8<wchar_t> convert_typeX;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.to_bytes(wstr);
    }

    void toStandard(std::string &str)
    {
        std::wstring ABC = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::wstring abc = L"абвгдеёжзийклмнопрстуфхцчшщъыьэюяabcdefghijklmnopqrstuvwxyz";

        std::wstring wstr = s2ws(str);
        for (size_t i = 0; i < wstr.size(); ++i)
        {
            if (!iswalnum(wstr[i]) && wstr[i] != '-' && (ABC + abc).find(wstr[i]) == std::string::npos)
            {
                wstr.erase(wstr.begin() + i);
                --i;
            }
            else
            {
                size_t found = ABC.find(wstr[i]);
                if (found != std::string::npos)
                {
                    wstr[i] = abc[found];
                }
            }
        }
        str = ws2s(wstr);
    }

    void delPunctuation(std::string &str)
    {
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (ispunct(str[i]) && str[i] != '-')
            {
                str.erase(str.begin() + i);
                --i;
            }
        }
    }

    std::unordered_set<std::string> getStopWords()
    {
        std::unordered_set<std::string> set;
        std::string fileName = "data/ru_stopwords_from_voyant.txt";
        std::ifstream fin(fileName);
        std::string word;
        while (!fin.eof())
        {
            fin >> word;
            set.insert(word);
        }
        fin.close();
        return set;
    }


    bool goodWord(std::string const & word)
    {
        static std::unordered_set<std::string> stopWords = getStopWords();
        if (stopWords.count(word) > 0)
        {
            return false;
        }
        return true;
    }

} // namespace utils
} //namespace mas
