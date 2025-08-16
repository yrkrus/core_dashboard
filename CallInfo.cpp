#include "CallInfo.h"
#include "Constants.h"
#include "InternalFunction.h"
#include <nlohmann/json.hpp>

using namespace utils;
using json = nlohmann::json;

CallInfo::CallInfo()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::CALL_INFO)
{
    // найдем не обработанные звонки 
    CreateListPhone();
}

CallInfo::~CallInfo()
{
}


void CallInfo::Execute()
{
    if (!IsExistList()) 
    {
        return;
    }

    FindInfoCall();
}

void CallInfo::CreateListPhone()
{
    std::string error;
    if (!GetInfoCallList(m_listPhone,error)) 
    {
        m_log.ToFile(ecLogType::eError, error);
    }
}

bool CallInfo::GetInfoCallList(InfoCallList &_list, std::string &_errorDescription)
{
    _list.clear();
    _errorDescription.clear();

    const std::string query = "select id,phone from ivr where operator is NULL and region is NULL";
    //const std::string query = "select id,phone,date_time from history_ivr where operator is NULL and region is NULL";
	
	if (!m_sql->Request(query, _errorDescription))
	{		
		_errorDescription += METHOD_NAME + StringFormat("query \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Info call;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
                case 0:	call.id = std::atoi(row[i]); break; 
                case 1:	call.phone = row[i]; break;
                case 2:	call.date = row[i]; break;                
			}
		}
		_list.push_back(call);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

    m_log.ToPrint(StringFormat("_list.size = %u",_list.size()));

	return true;
}

bool CallInfo::IsExistList()
{
    return !m_listPhone.empty();
}

void CallInfo::FindInfoCall()
{
       
    for (auto &call : m_listPhone) 
    {  
        std::string responce;
        std::string error;

        std::string request = GetLinkHttpRequest(call.phone);

        if (!Get(request, responce, error)) 
        {
            m_log.ToFile(ecLogType::eError,error);
            continue;
        }

        // разберем что пришло в ответе
        json j = json::parse(responce);
        call.phone_operator = j["operator"];
        call.region = j["region"];  
        
        if (!call.Check()) 
        {
            continue;
        }

        // занесем в БД
        UpdateToBaseInfoCall(call.id, call); 
        Sleep(10);  

    }
}

std::string CallInfo::GetLinkHttpRequest(const std::string &_phone)
{
    std::string replacment = HTTP_REQUEST;
    
    size_t position = replacment.find("%phone");
	replacment.replace(position, replacment.length(), _phone);
    
    return replacment;
}

void CallInfo::UpdateToBaseInfoCall(int _id, const Info &_call)
{
    std::string error;
     const std::string query = "update ivr set operator  = '" + _call.phone_operator
                                         + "' , region = '" + _call.region                                        
                                         + "' where id = '" + std::to_string(_id) + "'";
    //  const std::string query = "update history_ivr set operator  = '" + _call.phone_operator
    //                                      + "' , region = '" + _call.region                                        
    //                                      + "' where id = '" + std::to_string(_id) + "'";


    


	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);

		m_sql->Disconnect();
		return;
	}    

    m_log.ToPrint(StringFormat("%s | %s", _call.date.c_str(), query.c_str()));

	m_sql->Disconnect();
}

bool CallInfo::Get(const std::string &_request, std::string &_responce, std::string &_errorDescription)
{
    if (!IHTTPRequest::Get(_request, _responce))
    {
        _errorDescription = METHOD_NAME + _responce;
        return false;
    }

    return true;
}
