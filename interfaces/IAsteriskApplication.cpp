#include "IAsteriskApplication.h"
#include "../utils/InternalFunction.h"
#include "../system/Constants.h"

#define CHANNELS_FIELDS 14			        // кол-во полей при разборе начинается разбор с 0 ! итого 14 получиться
#define DELIMITER_CHANNELS_FIELDS '!'       // разделитель 

using utils::StringFormat;

namespace utils
{
    bool ParsingAsteriskRawDataRequest(std::vector<std::string> &_lines, const std::string &_rawLines, std::string &_errorDescription)
    {
        // предварительная проверка — ровно 13 (!) восклицательных знаков
        size_t nCountDelims = std::count(_rawLines.begin(), _rawLines.end(), DELIMITER_CHANNELS_FIELDS);

        if (nCountDelims != CHANNELS_FIELDS - 1)
        {
            _errorDescription = StringFormat("%s \t %s", METHOD_NAME, _rawLines.c_str());
            return false;
        }

        if (!utils::SplitDelimiterEntry(_rawLines, _lines, DELIMITER_CHANNELS_FIELDS, _errorDescription))
        {
            _errorDescription = StringFormat("%s \t %s", METHOD_NAME, _errorDescription.c_str());
            return false;
        }

        if (_lines.size() != CHANNELS_FIELDS || _lines.empty())
        {
            _errorDescription = StringFormat("%s \t %s", METHOD_NAME, _rawLines.c_str());
            return false;
        }

        return true;
    }

    void ShowVectorLines(const std::vector<std::string> &_lines)
    {
        for (size_t i = 0; i < _lines.size(); ++i)
        {
            printf("[%zu] %s\n", i, _lines[i].c_str());
        }
    }
} // end namespace utils
