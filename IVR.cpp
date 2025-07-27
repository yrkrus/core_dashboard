#include <vector>
#include <string>
#include "IVR.h"
#include "InternalFunction.h"
#include "Constants.h"

using namespace utils;

IVR::IVR()
	:IAsteriskData(CONSTANTS::TIMEOUT::IVR)
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
			// удал€ем из сырых данных
			DeleteRawLastData();
			return;
		}
	}

	// что то есть нужно теперь в Ѕƒ запихнуть
	if (IsExistListIvr())
	{		
		InsertIvrCalls();
		
		// удал€ем из сырых данных
		DeleteRawLastData();
	}
}

bool IVR::CreateCallers(const std::string &_lines, IvrCalls &_caller)
{
	bool status = false;
	
	std::vector<std::string> lines;
	std::string current_str;

	bool isNewLine = false;	
	
	for (size_t i = 0; i != _lines.length(); ++i)
	{
		if (isNewLine)
		{
			if (!current_str.empty())
			{
				lines.push_back(current_str);
				current_str.clear();
			}
		}

		if (_lines[i] != ' ') // ищем разделить (разделить пуста€ строка)
		{
			current_str += _lines[i];
			isNewLine = false;
		}
		else
		{
			isNewLine = true;
		}
	}

	if (!lines.empty())
	{
		// проверка на максимальное кол-во записей
		if (lines.size() != MAX_IVR_PARSING_LINES) 
		{
			/*LOG_old::LogToFile_old log(LOG_old::eLogType_ERROR);
			std::string err = std::string(__PRETTY_FUNCTION__) + "\t" + _lines;
			log.add(err);*/

			return false;
		}
		
		_caller.phone = utils::PhoneParsing(lines[7]);
		_caller.waiting = lines[8];
		_caller.callerID = StringToEnum(lines[0] + "," + lines[1]);
		
		// TODO тут в лог запись если не прошел по какой то причине 
		//std::string err = std::string(__PRETTY_FUNCTION__) + "\t" + _lines;
		if (!CheckCallers(_caller)) 
		{
			/*LOG_old::LogToFile_old log(LOG_old::eLogType_ERROR);
			err = std::string(__PRETTY_FUNCTION__) +"\t"+ _lines;
			log.add(err);*/

			return false;
		}	
		
		
		status = true;
	}

	return status;
}

bool IVR::CheckCallers(const IvrCalls &_caller)
{
	return _caller.check();
}

bool IVR::IsExistListIvr()
{
	return !m_listIvr.empty() ? true : false;
}

IVR::ECallerId IVR::StringToEnum(const std::string &_str)
{
	if (_str.find("ivr-13") != std::string::npos)			return ECallerId::Domru_220000;	
	if (_str.find("druOUT_220220") != std::string::npos)	return ECallerId::Domru_220220;	
	if (_str.find("Dru_220000") != std::string::npos)		return ECallerId::Domru_220220;
	if (_str.find("sts_") != std::string::npos)				return ECallerId::Sts;
	if (_str.find("221122") != std::string::npos)			return ECallerId::Comagic;
	if (_str.find("camaa") != std::string::npos)			return ECallerId::Comagic;
	if (_str.find("BeeIn") != std::string::npos)			return ECallerId::BeelineMih;
	
	return ECallerId::Unknown;
}

std::string IVR::EnumToString(ECallerId _caller)
{
	static std::map<ECallerId, std::string> callers =
	{
		{ECallerId::Unknown,		"Unknown"},
		{ECallerId::Domru_220220,	"220220"},
		{ECallerId::Domru_220000,	"220000"},
		{ECallerId::Sts,			"STS"},
		{ECallerId::Comagic,		"COMAGIC"},
		{ECallerId::BeelineMih,		"MIH" }
	};

	auto it = callers.find(_caller);
	if (it != callers.end()) 
	{
		return it->second;
	}

	return "Unknown";
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
			// номера такого нет нужно добавить в Ѕƒ
			const std::string query = "insert into ivr (phone,waiting_time,trunk) values ('" 
									+ list.phone + "','" 
									+ list.waiting + "','" 
									+ EnumToString(list.callerID) + "')";

			if (!m_sql->Request(query, _errorDescription))
			{
				_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
				m_log.ToFile(ELogType::Error, _errorDescription);

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
	const std::string query = "update ivr set waiting_time = '" 
										+ _caller.waiting 
										+ "' where phone = '" 
										+ _caller.phone 
										+ "' and id ='" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();
}


bool IVR::IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription)
{
	
	const std::string query = "select count(phone) from ivr where phone = '"
								+ std::string(_caller.phone) + "' and  date_time > '"
								+ GetCurrentDateTimeAfterMinutes(2) + "' and to_queue = '0' order by date_time desc";

	if (!m_sql->Request(query, _errorDescription))
	{	
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

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
								+ GetCurrentStartDay() + "' order by date_time desc limit 1";
	
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



