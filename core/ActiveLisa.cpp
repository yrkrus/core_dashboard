#include "ActiveLisa.h"
#include "../system/Constants.h"
#include "../utils/InternalFunction.h"
#include "../utils/custom_cast.h"

using namespace custom_cast;
using utils::StringFormat;
using utils::GetCurrentStartDay;


static std::string SIP_COMMANDS		        = "atsaero";	// поиск по этой строке
static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMANDS + "\"";

ActiveLisa::ActiveLisa()    
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::ACTIVE_CALLS_LISA))
{
}

ActiveLisa::~ActiveLisa()
{
}

bool ActiveLisa::Execute()
{
	// найдем данные
	std::string errorDescription;
	if (!CreateRawData(errorDescription))
	{
		//	m_log->ToFile(errorDescription);

		// на всякий случай обновим вдруг есть данные
		m_activeList.clear();
		UpdateCallSuccess();		
		return true;
	}

	// находим новые звонки
	FindNewCall();

	// обновляем звонки которые закончили разговаривать
	UpdateCallSuccess();

	return true;
}

bool ActiveLisa::CreateRawData(std::string &_errorDescription)
{
    return m_rawData.CreateData(SESSION_SIP_RESPONSE,_errorDescription);
}

bool ActiveLisa::CreateCallers(const std::string &_lines, ActiveLisaCall &_caller)
{
    std::vector<std::string> lines;
	std::string errorDescription;
	if (!utils::ParsingAsteriskRawDataRequest(lines, _lines, errorDescription))
	{
		return false;
	}	

	_caller.phone = utils::PhoneParsing(lines[7]);					// номер телефона
	_caller.phone_raw = lines[7];									// текущий номер телфеона с которым ведется беседа (сырой как по aster проходит)
	_caller.talkTime = static_cast<uint16_t>(std::stoi(lines[11])); // время развговора
	_caller.status = StringToEnum<ecAsteriskState>(lines[4]);		// статус
	_caller.call_id = lines[13];									// id звонка

	if (!CheckCallers(_caller))
	{
		errorDescription = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);
		return false;
	}	

	return true;
}

bool ActiveLisa::CheckCallers(const ActiveLisaCall &_caller)
{
	return _caller.check();
}

void ActiveLisa::FindNewCall()
{
	m_activeList.clear(); // обнулим текущий список

    std::string rawLines = m_rawData.GetRawLast(); 
    if (rawLines.empty())
    {
        return;
    }

    std::istringstream ss(rawLines);
    std::string line;

    while (std::getline(ss, line))
    {
         ActiveLisaCall caller;

        if (CreateCallers(line, caller))
        {
            m_activeList.push_back(caller);
        }
        else
        {
            // удаляем из сырых данных
            m_rawData.DeleteRawLast();
            break;
        }
    }

    // что то есть нужно теперь в БД запихнуть
    if (IsExistListLisa())
    {
        InsertLisaCalls();

        // удаляем из сырых данных
        m_rawData.DeleteRawLast();
    }		
}

bool ActiveLisa::IsExistListLisa()
{
    return !m_activeList.empty() ? true : false;
}

void ActiveLisa::InsertLisaCalls()
{
	if (!IsExistListLisa()) 
	{
		//errorDescription = StringFormat("%s ivr list is empty", METHOD_NAME);
		//printf("%s", errorDescription.c_str());
		return;
	}	

	for (const auto &call : m_activeList) 
	{
		std::string errorDescription;				
		bool errorConnectSQL = false;

		// проверим есть ли такой номер в бд
		if (IsExistCallLisa(call, errorDescription, errorConnectSQL)) 
		{
			if (errorConnectSQL)
			{
				continue;
			}
			
			uint32_t id;
			if (!GetID(call.phone, call.call_id, id)) 
			{
				continue;
			}		

			UpdateLisaCalls(id, call);
		}
		else  
		{ 
			if (errorConnectSQL) 
			{
				continue;
			}
			
			// номера такого нет нужно добавить в БД
			const std::string query = "insert into queue_lisa (phone, talk_time, call_id, answered) values ('" 
											+ call.phone + "', '" 
											+ std::to_string(call.talkTime) + "', '" 
											+ call.call_id + "', '"
											+ std::to_string(call.status == ecAsteriskState::Up ? 1 : 0) 
											+ "')";

			if (!m_sql->Request(query, errorDescription))
			{
				errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
				m_log->ToFile(ecLogType::eError, errorDescription);

				m_sql->Disconnect();
				
				continue;
			}

			m_sql->Disconnect();
		}
	}
}

bool ActiveLisa::IsExistCallLisa(const ActiveLisaCall &_caller, std::string &_errorDescription, bool &_errorConnectSQL)
{
	const std::string query = "select count(phone) from queue_lisa where phone = '" + std::string(_caller.phone) +
							  "' and call_id = '" + _caller.call_id + "'";

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		_errorConnectSQL = true;
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		
		_errorConnectSQL = true;
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		
		_errorConnectSQL = true;
		return false;
	}

	bool existLisaPhone;
	std::stoi(row[0]) == 0 ? existLisaPhone = false : existLisaPhone = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existLisaPhone;
}


bool ActiveLisa::GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id)
{
	std::string errorDescription;
	const std::string query = "select id from queue_lisa where phone = '" + _phone + 
							  "' and call_id = '" + _call_id + "'";
	
	if (!m_sql->Request(query))
	{
		// есть ошибка		
		m_sql->Disconnect();		
		return false;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr) 
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);	
		m_sql->Disconnect();

		return false;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr) 
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);		
		m_sql->Disconnect();

		return false;	
	}

	try
	{
		_id = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		return false;
	}	

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

void ActiveLisa::UpdateLisaCalls(uint32_t _id, const ActiveLisaCall &_caller)
{
	const std::string query = "update queue_lisa set talk_time = '" + std::to_string(_caller.talkTime) 
										+ "', answered = '" + std::to_string((_caller.status == ecAsteriskState::Up) ? 1 : 0) 
										+ "' where phone = '" + _caller.phone 
										+ "' and id = '" + std::to_string(_id) + "'";  
                                    
                                    

	std::string errorDescription;
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();
}

void ActiveLisa::UpdateCallSuccess()
{
	std::string errorDescription;
	if (!GetCallInBase(m_activeListBase, errorDescription)) 
	{		
		errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		return;	
	}

	if (m_activeListBase.empty())
	{
		return;
	}

	// обновляем
	for (const auto &call : m_activeListBase)
	{
		const std::string query = "update queue_lisa set hash = '" + std::to_string(call.hash)
									+ "' where id = '" + std::to_string(call.id) + "' and phone ='" + (call.phone) + "'";		

		if (!m_sql->Request(query, errorDescription))
		{
			errorDescription = StringFormat("%s\tquery \t%s", METHOD_NAME, query.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			m_sql->Disconnect();
			continue;
		}
		
		m_sql->Disconnect();
	}
}

bool ActiveLisa::GetCallInBase(ActiveLisaCallInBaseList &_callList, std::string &_errorDescription)
{	
	_callList.clear();
	
	const std::string query = "select id,phone,date_time,call_id from queue_lisa where date_time > '"
							+ GetCurrentStartDay()
							+ "' and hash is NULL";


	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		return false;
	}

	MYSQL_ROW row;
	
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		ActiveLisaCallInBase call;		

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	call.id = std::atoi(row[i]);	break;
				case 1:	call.phone = row[i];			break;
				case 2:	call.date_time = row[i];		break;
				case 3:	call.call_id = row[i];			break;
			}			
		}

		call.hash = std::hash<std::string>()(call.phone + "_" + call.date_time);		
		
		// проверка вдруг разговаривает еще
		if (!IsActiveTalk(call)) 
		{
			_callList.push_back(call);
		}		
	}

	mysql_free_result(result);
	m_sql->Disconnect();


	return true;	
}

bool ActiveLisa::IsActiveTalk(const ActiveLisaCallInBase &_call)
{
	bool status = false;	
	for (const auto &it : m_activeList)
	{
		if ((it.phone == _call.phone) && (it.call_id == _call.call_id))
		{
			status = true;
			break;
		}
	}

	return status;
}
