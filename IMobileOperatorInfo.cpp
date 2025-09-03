#include "IMobileOperatorInfo.h"

IMobileOperatorInfo::IMobileOperatorInfo(const std::string &_logName)
    : m_log(_logName) 
    , m_sql(std::make_shared<ISQLConnect>(false))
{
}

IMobileOperatorInfo::~IMobileOperatorInfo()
{
}

bool IMobileOperatorInfo::Execute()
{
    return false;
}
