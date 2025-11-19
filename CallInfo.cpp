// #include <nlohmann/json.hpp>

#include "CallInfo.h"
#include "system/Constants.h"
#include "utils/InternalFunction.h"
#include "utils/custom_cast.h"

using namespace utils;
using namespace custom_cast;

CallInfo::CallInfo(ecMobileInfoTable _table, const std::string &_logName)
    : MobileOperatorInfo(_table, _logName), m_table(_table)

{
}

CallInfo::~CallInfo()
{
}

bool CallInfo::Execute()
{
    return MobileOperatorInfo::Execute();
}

bool CallInfo::GetInfoMobileList(MobileInfoList &_list, std::string &_errorDescription)
{
    _list.clear();
    _errorDescription.clear();

    const std::string query = "select id, phone from " + EnumToString<ecMobileInfoTable>(m_table) 
                                                       + " where operator is NULL and region is NULL and trunk not in ('" + EnumToString<ecCallerId>(ecCallerId::InternalCaller) + "')";

    if (!MobileOperatorInfo::GetSQL()->Request(query, _errorDescription))
    {
        _errorDescription += METHOD_NAME + StringFormat("query \t%s", query.c_str());
        MobileOperatorInfo::GetLog()->ToFile(ecLogType::eError, _errorDescription);

        MobileOperatorInfo::GetSQL()->Disconnect();
        return false;
    }

    // результат
    MYSQL_RES *result = mysql_store_result(MobileOperatorInfo::GetSQL()->Get());
    if (result == nullptr)
    {
        _errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
        MobileOperatorInfo::GetLog()->ToFile(ecLogType::eError, _errorDescription);
        MobileOperatorInfo::GetSQL()->Disconnect();
        return false;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != NULL)
    {
        MobileInfo info;

        for (size_t i = 0; i < mysql_num_fields(result); ++i)
        {
            switch (i)
            {
            case 0:
                info.id = std::atoi(row[i]);
                break;
            case 1:
                info.phone = row[i];
                break;
            }
        }
        _list.push_back(info);
    }

    mysql_free_result(result);
    MobileOperatorInfo::GetSQL()->Disconnect();

    return true;
}