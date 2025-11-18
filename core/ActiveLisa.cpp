#include "ActiveLisa.h"
#include "../system/Constants.h"
#include "../utils/InternalFunction.h"
#include "../utils/custom_cast.h"

using namespace custom_cast;
using utils::StringFormat;


static std::string SIP_COMMANDS		        = "atsaero";	// поиск по этой строке
static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMANDS + "\"";

ActiveLisa::ActiveLisa()
    : IAsteriskData("ActiveLisa",CONSTANTS::TIMEOUT::ACTIVE_LISA)	
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::ACTIVE_LISA))
{
}

ActiveLisa::~ActiveLisa()
{
}

void ActiveLisa::Start()
{
    std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(SESSION_SIP_RESPONSE, error);
		};

	m_dispether.Start(func);	
}

void ActiveLisa::Stop()
{
    m_dispether.Stop();
	printf("ActiveLisa stopped!\n");
}

void ActiveLisa::Parsing()
{
    // установка hash когда закончили разговор
	if (IsExistListLisa()) 
	{
		
	}	
	
	m_activeList.clear(); // обнулим текущий список

    std::string rawLines = IAsteriskData::GetRawLastData();
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
            IAsteriskData::DeleteRawLastData();
            break;
        }
    }

    // что то есть нужно теперь в БД запихнуть
    if (IsExistListLisa())
    {
        InsertLisaCalls();

        // удаляем из сырых данных
        IAsteriskData::DeleteRawLastData();
    }	
}


bool ActiveLisa::CreateCallers(const std::string &_lines, ActiveLisaCall &_caller)
{
    // предварительная проверка — ровно 13 (!) восклицательных знаков
    size_t nCountDelims = std::count(_lines.begin(), _lines.end(), DELIMITER_CHANNELS_FIELDS);
    std::string error;

	if (nCountDelims != CHANNELS_FIELDS - 1)
    {
        std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, error);		
	
        return false;
    }
	
	std::vector<std::string> lines;
	if (!utils::SplitDelimiterEntry(_lines, lines, DELIMITER_CHANNELS_FIELDS, error)) 
	{
		error = StringFormat("%s \t %s", METHOD_NAME, error.c_str()); 
		m_log->ToFile(ecLogType::eError, error);		
		return false; 
	}
	
	if (lines.size() != CHANNELS_FIELDS || lines.empty()) 
	{
		std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, error);		
		return false;
	}

	//  for (const auto &it : lines)
	//  {
	//  	printf("%s\n",it.c_str());
	//  }

	_caller.phone = utils::PhoneParsing(lines[7]);					// номер телефона
	_caller.phone_raw = lines[7];									// текущий номер телфеона с которым ведется беседа (сырой как по aster проходит)
	_caller.talkTime = static_cast<uint16_t>(std::stoi(lines[11])); // время развговора
	_caller.status = StringToEnum<ecAsteriskState>(lines[4]);		// статус
	_caller.call_id = lines[13];									// id звонка

	if (!CheckCallers(_caller))
	{
		std::string errorDescription = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);
		return false;
	}	

	return true;
}

bool ActiveLisa::CheckCallers(const ActiveLisaCall &_caller)
{
	return _caller.check();
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
		
		// проверим есть ли такой номер в бд
		if (IsExistCallLisa(call, errorDescription)) 
		{
			uint32_t id;
			if (!GetID(call.phone, call.call_id, id)) 
			{
				continue;
			}		

			UpdateLisaCalls(id, call);
		}
		else  
		{ 
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

bool ActiveLisa::IsExistCallLisa(const ActiveLisaCall &_caller, std::string &_errorDescription)
{
	const std::string query = "select count(phone) from queue_lisa where phone = '" + std::string(_caller.phone) +
							  "' and call_id = '" + _caller.call_id + "'";

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existLisaPhone;
	std::stoi(row[0]) == 0 ? existLisaPhone = false : existLisaPhone = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existLisaPhone;
}


bool ActiveLisa::GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id)
{
	const std::string query = "select id from queue_lisa where phone = '" + _phone + 
							  "' and call_id = '" + _call_id + "'";
	
	if (!m_sql->Request(query))
	{
		// есть ошибка		
		m_sql->Disconnect();		
		return false;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	_id = std::stoi(row[0]);

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
