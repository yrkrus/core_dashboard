#include <vector>
#include <string>
#include "IVR.h"
#include "../utils/InternalFunction.h"
#include "../utils/custom_cast.h"
#include "../system/Constants.h"

using namespace custom_cast;
using utils::StringFormat;



// static std::string IVR_COMMANDS			= "Playback|lukoil|ivr-3";	// ищем только эти слова при формировании IVR
static std::string IVR_COMMANDS		=	"Playback";
// static std::string IVR_COMMANDS_EXT1	= "IVREXT";					// пропуск этой записи
 static std::string IVR_COMMANDS_EXT2	= "Spasibo";				// пропуск этой записи
// static std::string IVR_COMMANDS_EXT3	= "recOfficeOffline";		// пропуск этой записи
// static std::string IVR_COMMANDS_EXT4	= "noservice";				// пропуск этой записи 
// static std::string IVR_COMMANDS_EXT5	= "agent";					// пропуск этой записи 
// static std::string IVR_COMMANDS_EXT6	= "from-internal-xfer";		// пропуск этой записи (перевод звонка)

// static std::string IVR_COMMANDS_IK1 = "rec_IK_AllBusy";				// пропуск этой записи (IVR для ИК отдела)
// static std::string IVR_COMMANDS_IK2 = "rec_IK_Welcome";				// пропуск этой записи (IVR для ИК отдела)
// static std::string IVR_COMMANDS_IK3 = "rec_IK_WorkHours";			// пропуск этой записи (IVR для ИК отдела)

// static std::string IVR_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + IVR_COMMANDS + "\" " 
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT1 + "\" " 
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT3 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT4 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT5 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT6 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK1 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK2 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK3 + "\" ";

static std::string IVR_REQUEST = "asterisk -rx \"core show channels concise\" | grep -E \"" + IVR_COMMANDS + "\" "
																		  + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" ";


IVR::IVR()
	:IAsteriskData("IVR",CONSTANTS::TIMEOUT::IVR)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::IVR))
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
	
	std::string rawLines = IAsteriskData::GetRawLastData();
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
			IAsteriskData::DeleteRawLastData();
			break;
		}
	}

	// что то есть нужно теперь в БД запихнуть
	if (IsExistListIvr())
	{		
		InsertIvrCalls();
		
		// удаляем из сырых данных
		IAsteriskData::DeleteRawLastData();
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
	
		_caller.channel = lines[0];         										// имя канала
		_caller.context = lines[1];       											// диалплан‐контекст
		_caller.extension = lines[2];      											// номер(или “s”)
		_caller.priority = static_cast<uint16_t>(std::stoi(lines[3])); 	      		// приоритет
		_caller.state = StringToEnum<ecAsteriskState>(lines[4]);					// статус
		_caller.application = StringToEnum<ecAsteriskApp>(lines[5]);    			// текущее приложение(Dial, Playback, …)
		_caller.data = lines[6];             										// параметры приложения
		_caller.callerID = StringToEnum<ecCallerId>(lines[0]+" "+lines[6]);			// caller ID 
		
		_caller.callerID != ecCallerId::InternalCaller ? _caller.phone = utils::PhoneParsing(lines[7])			// номер телефона
													   : _caller.phone = utils::PhoneParsingInternal(lines[7]); 

		_caller.AMAFlags = lines[8];      											// AMA флаги(другие флаги ведения учёта)
		_caller.duration = !lines[9].empty() ? static_cast<uint16_t>(std::stoi(lines[9])) : 0; // время жизни канала в секундах
		_caller.bridgedChannel = lines[10]; 										// имя “связного” канала, если есть(иначе пусто)
		_caller.bridgedDuration = static_cast<uint16_t>(std::stoi(lines[11]));		//время “сращивания”(в секундах)
		_caller.uniqueID = lines[12];         										//уникальный идентификатор сессии
		_caller.call_id = lines[13];        										//идентификатор “корневого” звонка(call - trace)
		_caller.queue = FindQueue(_caller.callerID);								// очередь в которую должен попасть звонок
		
		if (!CheckCallers(_caller)) 
		{
			std::string errorDescription = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);
			return false;
		}	

		// if (_caller.bridgedDuration > 30) 
		// {
		// 	printf("\n%s\n",_lines.c_str());			
		// } 

	 return true;
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
	if (!IsExistListIvr()) 
	{
		//errorDescription = StringFormat("%s ivr list is empty", METHOD_NAME);
		//printf("%s", errorDescription.c_str());
		return;
	}	

	for (const auto &call : m_listIvr) 
	{
		std::string errorDescription;

		// проверим не повторный ли это звонок (очередь -> лиза -> очередь и т.д.)
		// для ИК техподдержки это не нужно
		if (call.queue != ecQueueNumber::e5911) 
		{
			if (call.bridgedDuration > m_time.time(call.queue)) 
			{
				// повторный звонок, кидаем его в ivr_loop
				IvrLoop(call);
				continue;
			}	
		} 			
		
		// проверим есть ли такой номер в бд
		if (IsExistCallIvr(call, errorDescription)) 
		{
			uint32_t id;
			if (!GetID(call.phone, call.call_id, id)) 
			{
				continue;
			}		

			UpdateIvrCalls(id, call);
		}
		else  
		{ 
			// номера такого нет нужно добавить в БД
			const std::string query = "insert into ivr (phone,call_time,trunk,call_id,number_queue) values ('" 
									+ call.phone + "','" 
									+ std::to_string(call.bridgedDuration) + "','" 
									+ EnumToString<ecCallerId>(call.callerID) + "','"
									+ call.call_id + "','"
									+ EnumToString<ecQueueNumber>(call.queue) + "')";

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

void IVR::UpdateIvrCalls(uint32_t _id, const IvrCalls &_caller)
{ 	
	const std::string query = "update ivr set call_time = '" 
										+ std::to_string(_caller.bridgedDuration) 
										+ "' where phone = '" 
										+ _caller.phone 
										+ "' and id ='" + std::to_string(_id) + "'";

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


bool IVR::IsExistCallIvr(const IvrCalls &_caller, std::string &_errorDescription)
{	
	const std::string query = "select count(phone) from ivr where phone = '" + std::string(_caller.phone) + 
							  "' and call_id = '"+_caller.call_id+"'";

	if (!m_sql->Request(query, _errorDescription))
	{	
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		// ошибка считаем  что есть запись
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		// ошибка считаем  что есть запись
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		// ошибка считаем  что есть запись
		return true;
	}

	bool existIvrPhone;
	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existIvrPhone;
}

void IVR::IvrLoop(const IvrCalls &_caller)
{
	if (!IsExistCallIvrLoop(_caller))
	{
		InsertIvrLoop(_caller);
	}
	else
	{
		uint32_t id;
		if (GetIDLoop(_caller.phone, _caller.call_id, id))
		{
			UpdateIvrLoop(_caller, id);
		}
	}
}

bool IVR::IsExistCallIvrLoop(const IvrCalls &_caller)
{
	const std::string query = "select count(phone) from ivr_loop where phone = '" + std::string(_caller.phone) + 
							  "' and call_id = '" + _caller.call_id + "'";

	std::string errorDescription;
	if (!m_sql->Request(query, errorDescription))
	{	
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		return true;
	}

	// результат
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

	bool existIvrPhone;
	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existIvrPhone;	
}

void IVR::InsertIvrLoop(const IvrCalls &_caller)
{
	const std::string query = "insert into ivr_loop (phone,call_time,call_id) values ('" 
							+ _caller.phone + "','" 
							+ std::to_string(_caller.bridgedDuration) + "','"						
							+ _caller.call_id+ "')";

	std::string _errorDescription;
	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);		
	}

	m_sql->Disconnect();
}

void IVR::UpdateIvrLoop(const IvrCalls &_caller, int _id)
{
	const std::string query = "update ivr_loop set call_time = '" 
										+ std::to_string(_caller.bridgedDuration) 
										+ "' where phone = '" 
										+ _caller.phone 
										+ "' and id ='" + std::to_string(_id) + "'";

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

bool IVR::GetIDLoop(const std::string &_phone, const std::string &_call_id, uint32_t &_id)
{
    std::string errorDescription;
	const std::string query = "select id from ivr_loop where phone = '" + _phone + 
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
		std::string errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		return false;
	}	

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

bool IVR::GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id)
{
	std::string errorDescription;
	const std::string query = "select id from ivr where phone = '" + _phone + 
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
		std::string errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		return false;
	}	
	

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

ecQueueNumber IVR::FindQueue(ecCallerId _caller)
{
    switch (_caller)
	{
		case ecCallerId::Domru_220220:
		case ecCallerId::Sts:
		case ecCallerId::Comagic:
		case ecCallerId::BeelineMih: return ecQueueNumber::e5000; break;
		case ecCallerId::Domru_220000: return ecQueueNumber::e5050; break;
		case ecCallerId::InternalCaller: return ecQueueNumber::e5911; break;
		default:
			return ecQueueNumber::eUnknown; break;

	}
}

void IVRTime::CreateMap(ecQueueNumber _queue)
{
	uint16_t time = GetTime(_queue);	
	m_value.insert(std::make_pair(_queue, time));
}

uint16_t IVRTime::GetTime(ecQueueNumber _queue)
{
    std::string queue;

	switch (_queue)
	{
		case ecQueueNumber::e5000: 	queue = "queue_5000_time"; break;
		case ecQueueNumber::e5050:	queue = "queue_5050_time"; break;

		case ecQueueNumber::e5000_e5050:	// не используется
		case ecQueueNumber::e5005:			// не используется
		case ecQueueNumber::e5911:	queue = "queue_5911_time"; break;			
		case ecQueueNumber::eUnknown:		// не используется
		return 0;		
	}
	
	const std::string query = "select "+ queue +" from settings order by date_time DESC limit 1";
	
	if (!m_sql->Request(query))
	{
		// есть ошибка		
		m_sql->Disconnect();		
		return false;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	int time = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return time;
}

IVRTime::IVRTime()
	: m_sql(std::make_shared<ISQLConnect>(false))
{
	CreateMap(ecQueueNumber::e5000);
	CreateMap(ecQueueNumber::e5050);
	CreateMap(ecQueueNumber::e5911);
}

IVRTime::~IVRTime()
{
}

uint16_t IVRTime::time(ecQueueNumber _queue)
{
   for (const auto &it: m_value) 
   {
		if (it.first == _queue) 
		{
			return it.second;
		}
   }

   return 0;
}
