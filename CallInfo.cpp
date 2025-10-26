#include <nlohmann/json.hpp>

#include "CallInfo.h"
#include "system/Constants.h"
#include "utils/InternalFunction.h"
#include "utils/custom_cast.h"


using namespace utils;
using namespace custom_cast;
using json = nlohmann::json;

CallInfo::CallInfo()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::CALL_INFO)
    , m_table(ecCallInfoTable::eIVR)  
{   
}

CallInfo::CallInfo(ecCallInfoTable _table)
    : m_sql(std::make_shared<ISQLConnect>(false)) 
    , m_log(CONSTANTS::LOG::CALL_INFO)
    , m_table(_table)
{
}

CallInfo::~CallInfo()
{
}


bool CallInfo::Execute()
{
    // найдем не обработанные звонки 
    CreateListPhone();
    
    if (IsExistList()) 
    {
        FindInfoCall();
    }

    return true;  
}

void CallInfo::CreateListPhone()
{
    std::string error;
    if (!GetInfoCallList(m_listPhone,error)) 
    {
        m_log.ToFile(ecLogType::eError, error);
    }
}

bool CallInfo::GetInfoCallList(InfoCallList_old &_list, std::string &_errorDescription)
{
    _list.clear();
    _errorDescription.clear();

    const std::string query = "select id, phone from "+EnumToString<ecCallInfoTable>(m_table)+" where operator is NULL and region is NULL and trunk not in ('"+EnumToString<ecCallerId>(ecCallerId::InternalCaller)+"')";
    	
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
		InfoCall_old call;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
                case 0:	call.id = std::atoi(row[i]); break; 
                case 1:	call.phone = row[i]; break;                             
			}
		}
		_list.push_back(call);
	}

	mysql_free_result(result);
	m_sql->Disconnect();  

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
        // проверим что этого номера нет в списке с ошибками
        if (m_listError.IsExistCall(call)) 
        {
            continue;
        }        
        
        std::string responce;
        std::string error;

        std::string request = GetLinkHttpRequest(call.phone);

        if (!Get(request, responce, error)) 
        {
            m_log.ToFile(ecLogType::eError,error);

            // добавим в список чтобы больше не проверять этот номер
            m_listError.Add(call);
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
    std::string replacment = HTTP_REQUEST_CALL;
    
    size_t position = replacment.find("%phone");
	replacment.replace(position, replacment.length(), _phone);
    
    return replacment;
}

void CallInfo::UpdateToBaseInfoCall(int _id, const InfoCall_old &_call)
{
    std::string error;
     const std::string query = "update " + EnumToString<ecCallInfoTable>(m_table) +" set operator  = '" + _call.phone_operator
                                                                                + "' , region = '" + _call.region                                        
                                                                                + "' where id = '" + std::to_string(_id) + "'";
    
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);

		m_sql->Disconnect();
		return;
	}      

	m_sql->Disconnect();
}

bool CallInfo::Get(const std::string &_request, std::string &_responce, std::string &_errorDescription)
{
    if (!IHTTPRequest::Get(_request, _responce))
    {
        _errorDescription = StringFormat("%s\t%s", METHOD_NAME, _responce.c_str());
        return false;
    }

    return true;
}

CallInfoError::CallInfoError()
{
}

CallInfoError::~CallInfoError()
{
}

bool CallInfoError::IsExistCall(const InfoCall_old &_call)
{
    for (const auto &_err : m_list) 
    {
        if (_err.phone == _call.phone) 
        {
            return true;
        } 
    }

    return false;
}

void CallInfoError::Add(const InfoCall_old &_call)
{
    m_list.emplace_back(_call);
}
