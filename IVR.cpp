#include <vector>
#include <string>
#include "IVR.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "custom_cast.h"

using namespace custom_cast;
using utils::StringFormat;


IVR::IVR()
	:IAsteriskData("IVR",CONSTANTS::TIMEOUT::IVR)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::IVR)
{
}

IVR::~IVR()
{
}

void IVR::Start()
{
	std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(IVR_REQUEST, error);
		};

	m_dispether.Start(func);
}

void IVR::Stop()
{
	m_dispether.Stop();
	printf("IVR stopped!\n");
}


void IVR::Parsing()
{
	m_listIvr.clear(); // обнулим текущий список	
	
	std::string rawLines = GetRawLastData();
	if (rawLines.empty()) 
	{
		return;
	}	

	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))	
	{
		IvrCalls caller;

		if (CreateCallers(line, caller)) 
		{
			m_listIvr.push_back(caller);
		}
		else 		
		{
			// удаляем из сырых данных
			DeleteRawLastData();
			break;
		}
	}

	// что то есть нужно теперь в БД запихнуть
	if (IsExistListIvr())
	{		
		InsertIvrCalls();
		
		// удаляем из сырых данных
		DeleteRawLastData();
	}
}

bool IVR::CreateCallers(const std::string &_lines, IvrCalls &_caller)
{	
	// предварительная проверка — ровно 13 (!) восклицательных знаков
    size_t nCountDelims = std::count(_lines.begin(), _lines.end(), DELIMITER_CHANNELS_FIELDS);
    std::string error;

	if (nCountDelims != CHANNELS_FIELDS - 1)
    {
        std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log.ToFile(ecLogType::eError, error);		
	
        return false;
    }
	
	std::vector<std::string> lines;
	if (!utils::SplitDelimiterEntry(_lines, lines, DELIMITER_CHANNELS_FIELDS, error)) 
	{
		error = StringFormat("%s \t %s", METHOD_NAME, error.c_str()); 
		m_log.ToFile(ecLogType::eError, error);		
		return false; 
	}
	
	if (lines.size() != CHANNELS_FIELDS || lines.empty()) 
	{
		std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log.ToFile(ecLogType::eError, error);		
		return false;
	}  

	 for (size_t i=0; i<lines.size(); ++i) 
	 {
	 	printf("\n%zu. value = %s", i, lines[i].c_str());	
	 }	
	
		_caller.channel = lines[0];         										// имя канала
		_caller.context = lines[1];       											// диалплан‐контекст
		_caller.extension = lines[2];      											// номер(или “s”)
		_caller.priority = static_cast<uint16_t>(std::stoi(lines[3])); 	      		// приоритет
		_caller.state = StringToEnum<ecAsteriskState>(lines[4]);					// статус
		_caller.application = StringToEnum<ecAsteriskApp>(lines[5]);    			// текущее приложение(Dial, Playback, …)
		_caller.data = lines[6];             										// параметры приложения
		_caller.callerID = StringToEnum<ecCallerId>(lines[0]+" "+lines[6]);			// caller ID
		_caller.phone = utils::PhoneParsing(lines[7]);								// номер телефона
		_caller.AMAFlags = lines[8];      											// AMA флаги(другие флаги ведения учёта)
		_caller.duration = !lines[9].empty() ? static_cast<uint16_t>(std::stoi(lines[9])) : 0; // время жизни канала в секундах
		_caller.bridgedChannel = lines[10]; 										// имя “связного” канала, если есть(иначе пусто)
		_caller.bridgedDuration = static_cast<uint16_t>(std::stoi(lines[11]));		//время “сращивания”(в секундах)
		_caller.uniqueID = lines[12];         										//уникальный идентификатор сессии
		_caller.call_id = lines[13];        										//идентификатор “корневого” звонка(call - trace)

		
	// 	_caller.phone = utils::PhoneParsing(lines[7]);
	// 	_caller.waiting = lines[8];
	// 	_caller.callerID = StringToEnum(lines[0] + "," + lines[1]);
		
	// 	if (!CheckCallers(_caller)) 
	// 	{
	// 		std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
	// 		m_log.ToFile(ecLogType::eError, error);

	// 		return false;
	// 	}	
		
	// 	status = true;
	// }

	 return false;
}

bool IVR::CheckCallers(const IvrCalls &_caller)
{
	return _caller.check();
}

bool IVR::IsExistListIvr()
{
	return !m_listIvr.empty() ? true : false;
}



void IVR::InsertIvrCalls()
{
	std::string _errorDescription = "";

	if (!IsExistListIvr()) 
	{
		_errorDescription = StringFormat("%s ivr list is empty", METHOD_NAME);
		printf("%s", _errorDescription.c_str());
		return;
	}	

	for (const auto &list : m_listIvr) 
	{
		// проверим есть ли такой номер в бд
		if (IsExistIvrPhone(list, _errorDescription)) 
		{
			int id = GetPhoneIDIvr(list.phone);
			if (id <= 0) 
			{				
				continue;
			}

			UpdateIvrCalls(id, list);
		}
		else  
		{ 
			// номера такого нет нужно добавить в БД
			const std::string query = "insert into ivr (phone,call_time,trunk) values ('" 
									+ list.phone + "','" 
									+ std::to_string(list.bridgedDuration) + "','" 
									+ EnumToString<ecCallerId>(list.callerID) + "')";

			if (!m_sql->Request(query, _errorDescription))
			{
				_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
				m_log.ToFile(ecLogType::eError, _errorDescription);

				m_sql->Disconnect();
				
				continue;
			}

			m_sql->Disconnect();
		}
	}
}

void IVR::UpdateIvrCalls(int _id, const IvrCalls &_caller)
{
	std::string error;
	const std::string query = "update ivr set call_time = '" 
										+ std::to_string(_caller.bridgedDuration) 
										+ "' where phone = '" 
										+ _caller.phone 
										+ "' and id ='" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);

		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();
}


bool IVR::IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription)
{
	
	const std::string query = "select count(phone) from ivr where phone = '"
								+ std::string(_caller.phone) + "' and  date_time > '"
								+ utils::GetCurrentDateTimeAfterMinutes(2) + "' and to_queue = '0' order by date_time desc";

	if (!m_sql->Request(query, _errorDescription))
	{	
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existIvrPhone;
	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existIvrPhone;

}

int IVR::GetPhoneIDIvr(const std::string &_phone)
{
	const std::string query = "select id from ivr where phone = "
								+ _phone + " and date_time > '"
								+ utils::GetCurrentStartDay() + "' order by date_time desc limit 1";
	
	if (!m_sql->Request(query))
	{
		// есть ошибка		
		m_sql->Disconnect();
		return -1;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	int id = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return id;
}



