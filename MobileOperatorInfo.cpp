
#include <nlohmann/json.hpp>

#include "MobileOperatorInfo.h"
#include "system/Constants.h"
#include "utils/InternalFunction.h"
#include "utils/custom_cast.h"

// TODO на это заменить  errorDesciption = StringFormat("%s\tInitAuth SMS error\t%s",METHOD_NAME,errorDesciption.c_str());

using namespace utils;
using namespace custom_cast;
using json = nlohmann::json;


MobileOperatorInfo::MobileOperatorInfo(ecMobileInfoTable _table, const::std::string &_logName)
    : m_sql(std::make_shared<ISQLConnect>(false)) 
    , m_log(std::make_shared<Log>(_logName))
    , m_table(_table)
{
}

MobileOperatorInfo::~MobileOperatorInfo()
{
}


bool MobileOperatorInfo::Execute()
{
    // найдем не обработанные звонки 
    CreateListPhone();
    
    if (IsExistList()) 
    {
        FindInfoOperatorAndRegion();
    }

    return true;  
}

SP_SQL MobileOperatorInfo::GetSQL()
{
    return m_sql;
}

SP_Log MobileOperatorInfo::GetLog()
{
    return m_log;
}

void MobileOperatorInfo::CreateListPhone()
{
    std::string error;
    if (!GetInfoMobileList(m_listPhone,error)) 
    {
        m_log->ToFile(ecLogType::eError, error);
    }
}

bool MobileOperatorInfo::IsExistList()
{
    return !m_listPhone.empty();
}

void MobileOperatorInfo::FindInfoOperatorAndRegion()
{       
    for (auto &info : m_listPhone) 
    {  
        // проверим что этого номера нет в списке с ошибками
        if (m_listError.IsExistPhone(info)) 
        {
            continue;
        }        
        
        std::string responce;
        std::string error;

        std::string request = GetLinkHttpRequest(info.phone);

        if (!Get(request, responce, error)) 
        {
            m_log->ToFile(ecLogType::eError,error);

            // добавим в список чтобы больше не проверять этот номер
            m_listError.Add(info);
            continue;
        }

        // разберем что пришло в ответе
        json j = json::parse(responce);
        info.phone_operator = j["operator"];
        info.region = j["region"];  
        
        if (!info.Check()) 
        {
            continue;
        }

        // занесем в БД
        UpdateToBaseInfo(info.id, info); 
        Sleep(10); 
    }
}

std::string MobileOperatorInfo::GetLinkHttpRequest(const std::string &_phone)
{
    std::string replacment = HTTP_REQUEST_INFO;
    
    size_t position = replacment.find("%phone");
	replacment.replace(position, replacment.length(), _phone);
    
    return replacment;
}

void MobileOperatorInfo::UpdateToBaseInfo(int _id, const MobileInfo &_call)
{
    std::string error;
     const std::string query = "update " + EnumToString<ecMobileInfoTable>(m_table) +" set operator  = '" + _call.phone_operator
                                                                                + "' , region = '" + _call.region                                        
                                                                                + "' where id = '" + std::to_string(_id) + "'";
    
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();
		return;
	}      

	m_sql->Disconnect();
}

bool MobileOperatorInfo::Get(const std::string &_request, std::string &_responce, std::string &_errorDescription)
{
    if (!IHTTPRequest::Get(_request, _responce))
    {
        _errorDescription = StringFormat("%s\t%s", METHOD_NAME, _responce.c_str());
        return false;
    }

    return true;
}

MobileInfoError::MobileInfoError()
{
}

MobileInfoError::~MobileInfoError()
{
}

bool MobileInfoError::IsExistPhone(const MobileInfo &_info)
{
    for (const auto &_err : m_list) 
    {
        if (_err.phone == _info.phone) 
        {
            return true;
        } 
    }

    return false;
}

void MobileInfoError::Add(const MobileInfo &_info)
{
    m_list.emplace_back(_info);
}

