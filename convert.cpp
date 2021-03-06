#include "convert.h"


namespace mas
{
namespace util
{
    void outToStream(std::ofstream & out, std::wstring & buffer)
    {
        boost::trim(buffer);
        if (!buffer.empty())
        {
            out << mas::utils::ws2s(buffer) << ' ';
            buffer.clear();
        }
    }

    void addFile(std::wifstream & fin,
                 std::ofstream & out,
                 const std::string & theme,
                 const std::string & fileName)
    {
        std::string firstLine = "TOPIC:" + theme + "_FILENAME:" + fileName + " X ";
        bool printFirstLine = false;

        std::wstring buffer;
        std::getline<wchar_t>(fin, buffer);
        boost::trim(buffer);

        std::set<std::wstring> ignoredTags = {L"Sentence="};

        if (!buffer.empty())
        {
            auto itTag = std::find_if(ignoredTags.begin(), ignoredTags.end(),
                            [&buffer](std::wstring tag){return buffer.compare(0, tag.size(), tag) == 0;});
            if (itTag != ignoredTags.end())
            {
                std::wstring smallBuf = buffer.substr(itTag->size());
                boost::trim(smallBuf);
                if (!smallBuf.empty())
                {
                    out << firstLine;
                    printFirstLine = true;
                }
                outToStream(out, smallBuf);
            }
            else
            {
                out << firstLine;
                printFirstLine = true;
                outToStream(out, buffer);
            }
        }

        while(std::getline<wchar_t>(fin, buffer))
        {
            if (!printFirstLine)
            {
                out << firstLine;
                printFirstLine = true;
            }
            outToStream(out, buffer);
        }
        if (printFirstLine)
        {
            out << std::endl;
            out.flush();
        }
    }

} // namespace util


size_t convert(const std::string & inputPath, const std::string & outputPath)
{
    using namespace boost::filesystem;

    std::cout << "CONVERT " << inputPath << " STARTED ...\n";

    size_t numTopics = 0;

    try
    {
        path inputDir(inputPath);
        std::ofstream outputFile(outputPath);

        if (is_directory(inputDir))
        {
            for (auto nextDir = directory_iterator(inputDir); nextDir != directory_iterator(); ++nextDir)
            {
                if (is_directory(*nextDir))
                {
                    bool existTxtFile = false;

                    std::string theme = mas::utils::ws2s(nextDir->path().filename().wstring());
                    std::replace(theme.begin(), theme.end(), ' ', '_');

                    for (auto nextFile = directory_iterator(*nextDir); nextFile != directory_iterator(); ++nextFile)
                    {
                        if (is_regular_file(*nextFile) && nextFile->path().extension().string() == ".txt")
                        {
                            existTxtFile = true;
                            std::wifstream inputFile(nextFile->path().string());

                            std::locale cp1251(std::locale(""), new codecvt_cp1251); // for cp1251 files
                            inputFile.imbue(cp1251); // for cp1251 files
//                            inputFile.imbue(std::locale("")); // for utf-8 files

                            std::string fileName = mas::utils::ws2s(nextFile->path().stem().wstring());
                            std::replace(fileName.begin(), fileName.end(), ' ', '_');

                            util::addFile(inputFile, outputFile, theme, fileName);
                        }
                    }

                    if (existTxtFile) ++numTopics;
                }
            }
        }
        else
        {
            std::cout << inputDir << " does not exist or is nor a directory\n";
        }

        outputFile.close();
    }
    catch (const filesystem_error& ex)
    {
      std::cerr << ex.what() << '\n';
    }

    std::cout << "... CONVERT FINISHED (base contains " << numTopics << " folders) \n\n";

    return numTopics;
}


} // namespace mas

