#include <vector>
#include <string>
#include "Queue.h"
#include "../utils/InternalFunction.h"
#include "../utils/custom_cast.h"
#include "../system/Constants.h"

using namespace utils;
using namespace custom_cast;

static std::string SIP_COMMANDS_FND			= "!Up!Queue!";	// поиск по этой строке

static std::string SESSION_QUEUE_RESPONSE	= "asterisk -rx \"queue show %queue\"";
static std::string QUEUE_REQUEST 			= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMANDS_FND + "\"";


Queue::Queue()
	: IAsteriskData("Queue",CONSTANTS::TIMEOUT::QUEUE)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::QUEUE))
{
}

Queue::~Queue()
{
}

void Queue::Start()
{
	std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(QUEUE_REQUEST, error);
		};

	m_dispether.Start(func);
}

void Queue::Stop()
{
	m_dispether.Stop();
	printf("Queue stopped!\n");
}

void Queue::Parsing()
{	
	if (FindQueueCallers())	// ждем сырые данные 
	{		
		// есть данные добавляем\обновляем в БД
		InsertQueueCalls();

		// обновляем очереди (по списку)
		if (IsExistQueueCalls()) 
		{
			UpdateCalls(m_listQueue);
		}		
	}	

	// удаляем из сырых данных
	IAsteriskData::DeleteRawLastData();
}

bool Queue::FindQueueCallers()
{	
	m_listQueue.clear(); // обнулим текущий список
	
	std::string rawLines = IAsteriskData::GetRawLastData();
	if (rawLines.empty())
	{		
		return false;
	}		

	std::istringstream ss(rawLines);
	std::string line;
 
	while (std::getline(ss, line))
	{
		QueueCalls queueCaller;

		if (CreateQueueCallers(line, queueCaller))
		{
			m_listQueue.push_back(queueCaller);
		}
	}

	return IsExistQueueCalls();	
}

bool Queue::CreateQueueCallers(const std::string &_lines, QueueCalls &_queueCaller)
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

	//   for (const auto &it : lines)
	//   {
	//   	printf("%s\n",it.c_str());
	//   }

	_queueCaller.queue = StringToEnum<ecQueueNumber>(lines[2]);										  // номер очереди
	_queueCaller.state = StringToEnum<ecAsteriskState>(lines[4]);									  // текущее состояние канала (Up, Ring, Down и т.п.)
	_queueCaller.application = StringToEnum<ecAsteriskApp>(lines[5]);								  // текущее приложение(Dial, Playback, …)
	_queueCaller.phone = (_queueCaller.queue != ecQueueNumber::e5911) ? utils::PhoneParsing(lines[7]) // текущий номер телефона который в очереди сейчас
																	  : utils::PhoneParsingInternal(lines[7]);
	_queueCaller.waiting = static_cast<uint16_t>(std::stoi(lines[11])); // время ожидания
	_queueCaller.call_id = lines[13];									// id звонка (ivr)

	
	// TODO тут в лог запись если не прошел по какой то причине 
	if (!CheckCallers(_queueCaller))
	{
		std::string error = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, error);

		return false;
	}
	
	return true;
}

bool Queue::CheckCallers(const QueueCalls &_caller)
{
	return _caller.check();	
}

bool Queue::IsExistQueueCalls()
{
	return !m_listQueue.empty() ? true : false;
}

void Queue::InsertQueueCalls()
{	
	for (const auto &list : m_listQueue) 
	{	
		list.queue != ecQueueNumber::e5005	? InsertCall(list)						// для очереди из живых операторо											
											: InsertCallVirtualOperator(list);		// для виртуалной бабы				
	}	
}

void Queue::UpdateCalls(const QueueCallsList &_callList)
{
	/*if (!IsExistQueueCalls()) 
	{
		return;
	}*/
	
	// находим и обновляем данные если звонок был в очереди, но не дождался ответа от оператора
	UpdateCallFail(_callList);

	// находим и обновляем данные когда у нас звонок из IVR попал в очередь\на виртаульного оператора
	UpdateCallIvr(_callList);

	// находим и обновляем данные когда разговор успешно состоялся 
	UpdateCallSuccess(_callList);
}

void Queue::InsertCall(const QueueCalls &_call)
{
	std::string errorDescription;

	if (IsExistCall(_call)) 
	{
		// номер существует, обновляем данные
		int id = GetLastQueueCallId(_call.phone, _call.call_id);
		
		if (!UpdateCall(id, _call, errorDescription))
		{
			printf("%s", errorDescription.c_str());
			return;
		}		
	}
	else 
	{		
		// нет такого звонка добавляем
		const std::string query = "insert into queue (number_queue,phone,waiting_time,id_ivr) values ('"
									+ EnumToString(_call.queue) + "','"
									+ _call.phone + "','"
									+ GetTalkTime(_call.waiting) + "','"
									+ _call.call_id + "')";

		if (!m_sql->Request(query, errorDescription))
		{
			errorDescription = StringFormat("%s\tquery \t%s", METHOD_NAME, query.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			m_sql->Disconnect();						
		}
		
		m_sql->Disconnect();	
	}		
}

// добавление нового звонка (виртуальный оператор лиза)
void Queue::InsertCallVirtualOperator(const QueueCalls &_call)
{
	std::string error;

	if (IsExistCallVirtualOperator(_call.queue, _call.phone))
	{
		// номер существует, обновляем данные
		unsigned int id = GetLastQueueVirtualOperatorCallId(_call.phone);

		if (!UpdateCallVirualOperator(id, _call, error))
		{
			printf("%s", error.c_str());
		}
	}
	else
	{
		// нет такого звонка добавляем
		const std::string query = "insert into queue_robot (number_queue,phone,talk_time) values ('"
			+ EnumToString(_call.queue) + "','"
			+ _call.phone + "','"
			+  GetTalkTime(_call.waiting) + "')";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, error);

			m_sql->Disconnect();			
		}

		m_sql->Disconnect();
	}
}

bool Queue::UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription)
{
	const std::string query = "update queue set waiting_time = '" 
								+  GetTalkTime(_call.waiting) 
								+ "' where phone = '" 
								+ _call.phone 
								+ "' and id ='" + std::to_string(_id) + "'";;
	
	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription = StringFormat("%s\tquery \t%s", METHOD_NAME, query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();		
		return false;
	}
	m_sql->Disconnect();
	
	return true;
}

// обновление существующего звонка (виртуальный оператор)
bool Queue::UpdateCallVirualOperator(int _id, const QueueCalls &_call, std::string &_errorDescription)
{
	// TODO подумать может отказаться от этой таблицы теперь

	// std::string error;
	// const std::string query = "update queue_robot set talk_time = '"
	// 						+ _call.waiting
	// 						+ "' where phone = '"
	// 						+ _call.phone
	// 						+ "' and id ='" + std::to_string(_id) + "'";

	// if (!m_sql->Request(query, error))
	// {
	// 	error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
	// 	m_log->ToFile(ecLogType::eError, error);

	// 	m_sql->Disconnect();		
	// 	return false;
	// }
	// m_sql->Disconnect();

	return true;
}

void Queue::UpdateCallFail(const QueueCallsList &_calls)
{	
	// найдем текущие номера которые мы не будет трогать при обновлении
	std::string phoneDoNotTouch;
	
	// TODO вдруг пустой список, смысл проверять!?
	if (_calls.empty()) 
	{
		return;
	}

	for (const auto &list : _calls)
	{
		if (phoneDoNotTouch.empty())
		{
			phoneDoNotTouch = "'" + list.phone + "'";
		}
		else
		{
			phoneDoNotTouch += ",'" + list.phone + "'";	// TODO тут проверить 
		}
	}	
	
	// обновляем данные	
	const std::string query = "update queue set fail = '1' where date_time > '"
										+ GetCurrentStartDay()
										+ "' and answered = '0'"
										+ " and sip = '-1' and phone not in (" + phoneDoNotTouch + ")";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();		
		return;
	}
	m_sql->Disconnect();
}

//void Queue::UpdateCallFail()
//{
//	// обновляем данные
//	const std::string query = "update queue set fail = '1' where date_time > '" 
//								+ getCurrentDateTimeAfter20hours() 
//								+ "' and answered = '0' and sip = '-1' ";
//
//	if (!m_sql->Request(query))
//	{
//		m_sql->Disconnect();		
//		return;
//	}	
//
//	m_sql->Disconnect();
//}

	// обновление данных когда у нас звонок из IVR попал в очередь или на виртуального оператора
void Queue::UpdateCallIvr(const QueueCallsList &_calls)
{	
	if (_calls.empty()) return; // на всякий случай
	
	// звонок ушел на очередь с живыми операторами
	UpdateCallIvrToQueue(_calls);

	// звонок ушел на виртуального оператора
	UpdateCallIvrToVirtualOperator(_calls);	
}

// звонок из IVR попал в очередь
void Queue::UpdateCallIvrToQueue(const QueueCallsList &_calls)
{
	// найдем текущие номера которые будем трогать при обновлении
	std::string phoneTouch;

	for (const auto &call : _calls)
	{
		if (call.queue != ecQueueNumber::e5005) 
		{
			if (phoneTouch.empty())
			{
				phoneTouch = "'" + call.phone + "'";
			}
			else
			{
				phoneTouch += ",'" + call.phone + "'"; 
			}
		}		
	}

	if (phoneTouch.empty()) return;

	const std::string query = "update ivr set to_queue = '1' where date_time > '"
		+ GetCurrentDateTimeAfterMinutes(5)
		+ "' and phone in(" + phoneTouch + ") and to_queue = '0' and to_robot ='0' ";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();		
		return;
	}

	m_sql->Disconnect();
}

// звонок из IVR попал на виртуального оператора
void Queue::UpdateCallIvrToVirtualOperator(const QueueCallsList &_calls)
{
	// найдем текущие номера которые будем трогать при обновлении
	std::string phoneTouch;

	for (const auto &call : _calls)
	{
		if (call.queue == ecQueueNumber::e5005) 
		{
			if (phoneTouch.empty())
			{
				phoneTouch = "'" + call.phone + "'";
			}
			else
			{
				phoneTouch += ",'" + call.phone + "'"; 
			}
		}		
	}
	
	if (phoneTouch.empty()) return;

	const std::string query = "update ivr set to_robot = '1' where date_time > '"
								+ GetCurrentDateTimeAfterMinutes(5)
								+ "' and phone in(" + phoneTouch + ") and to_robot = '0' and to_queue = '0'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();		
		return;
	}

	m_sql->Disconnect();
}

void Queue::UpdateCallSuccess(const QueueCallsList &_calls)
{
	// разговор успешно состоялся реальный оператор
	UpdateCallSuccessRealOperator(_calls);

	// разговор успешно состоялся виртуальный оператор
	UpdateCallSuccessVirtualOperator(_calls);
}

// разговор успешно состоялся реальный оператор
void Queue::UpdateCallSuccessRealOperator(const QueueCallsList &_calls)
{
	CallsInBaseList callsInBase;

	std::string error;
	if (!GetCallsInBase(callsInBase, _calls, error))
	{
		return;
	}

	// обновляем	
	for (const auto &call : callsInBase)
	{
		std::string error;
		const std::string query = "update queue set hash = '" + std::to_string(call.hash)
															+ "' where id ='" + call.id
															+ "' and phone ='" + call.phone
															+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, error);

			m_sql->Disconnect();
			continue;
		}
	};

	m_sql->Disconnect();
}

// разговор успешно состоялся виртуальный оператор
void Queue::UpdateCallSuccessVirtualOperator(const QueueCallsList &_calls)
{
	CallsInBaseList callsInBase;

	std::string error;
	if (!GetCallsInBaseVirtualOperator(callsInBase, _calls, error))
	{
		return;
	}

	// обновляем	
	for (const auto &call : callsInBase)
	{
		std::string error;
		const std::string query = "update queue_robot set hash = '" + std::to_string(call.hash)
									+ "' where id ='" + call.id
									+ "' and phone ='" + call.phone
									+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, error);

			m_sql->Disconnect();
			continue;
		}
	};

	m_sql->Disconnect();
}

// обновление данных когда нет активных операторов на линии
void Queue::UpdateCallSuccess()
{
	if (IsExistAnyAnsweredCall())
	{
		UpdateAllAnyAnsweredCalls();

		QueueCallsList emptyList;
		UpdateCalls(emptyList);
	}	
}

bool Queue::IsExistCall(const QueueCalls &_call)
{
	std::string errorDescription;
	// правильней проверять сначало разговор	
	const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString<ecQueueNumber>(_call.queue)
								+ "' and phone = '" + _call.phone + "'"							
								+ " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL "
								+ " and id_ivr = '"+_call.call_id+"'";
	
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();		
		// при ошибке считаем что запись есть
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if(result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		return true;
	}

	unsigned int countPhone = std::stoi(row[0]);
	mysql_free_result(result);
	m_sql->Disconnect();

	if (countPhone >= 1)
	{
		return true;
	}
	else
	{
		// проверяем вдруг в очереди сейчас находится звонок
		const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString <ecQueueNumber>(_call.queue)
								+ "' and phone = '" + _call.phone + "'"
								+ " and answered ='0' and fail='0' and sip='-1' and hash is NULL"
								+ " and id_ivr = '"+_call.call_id+"'";

		if (!m_sql->Request(query, errorDescription))
		{
			errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			m_sql->Disconnect();			
			// при ошибке считаем что запись есть
			return true;
		}

		// результат
		MYSQL_RES *result = mysql_store_result(m_sql->Get());
		if (result == nullptr)
		{
			errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
			m_log->ToFile(ecLogType::eError, errorDescription);
			m_sql->Disconnect();
			// при ошибке считаем что запись есть
			return true;
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if(row == nullptr)
		{
			errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
			m_log->ToFile(ecLogType::eError, errorDescription);
			m_sql->Disconnect();
			// при ошибке считаем что запись есть
			return true;
		}

		int countPhone = std::stoi(row[0]);

		mysql_free_result(result);
		m_sql->Disconnect();

		if (countPhone >= 1)
		{
			return true;
		}
		else
		{
			// нет разговора проверяем повтрность
			const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString<ecQueueNumber>(_call.queue)
				+ "' and phone = '" + _call.phone + "'"			
				+ " and answered ='0' and fail='1' and sip = '-1' and hash is NULL"
				+ " and id_ivr = '"+_call.call_id+"'";
		
			if (!m_sql->Request(query, errorDescription))
			{
				errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
				m_log->ToFile(ecLogType::eError, errorDescription);

				m_sql->Disconnect();				
				// при ошибке считаем что запись есть
				return true;
			}

			// результат
			MYSQL_RES *result = mysql_store_result(m_sql->Get());
			if (result == nullptr)
			{
				errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
				m_log->ToFile(ecLogType::eError, errorDescription);
				m_sql->Disconnect();
				// при ошибке считаем что запись есть
				return true;
			}

			MYSQL_ROW row = mysql_fetch_row(result);
			if (row == nullptr)
			{
				errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
				m_log->ToFile(ecLogType::eError, errorDescription);
				m_sql->Disconnect();
				// при ошибке считаем что запись есть
				return true;
			}

			int countPhone = std::stoi(row[0]);

			mysql_free_result(result);
			m_sql->Disconnect();

			if (countPhone >= 1)
			{				
				return false; // считаем как новый вызов!!!
			}
			else
			{
				// проверка на повторность, вдруг еще раз перезвонили после того как поговорили уже	
				const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString<ecQueueNumber>(_call.queue)
					+ "' and phone = '" + _call.phone + "'"				
					+ " and answered = '1' and fail = '0' and sip <>'-1'"
					+ " and hash is not NULL"
					+ " and id_ivr = '" + _call.call_id + "'";				
				
				if (!m_sql->Request(query, errorDescription))
				{
					errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
					m_log->ToFile(ecLogType::eError, errorDescription);

					m_sql->Disconnect();					
					// при ошибке считаем что запись есть
					return true;
				}

				// результат
				MYSQL_RES *result = mysql_store_result(m_sql->Get());
				if (result == nullptr)
				{
					errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
					m_log->ToFile(ecLogType::eError, errorDescription);
					m_sql->Disconnect();
					// при ошибке считаем что запись есть
					return true;
				}

				MYSQL_ROW row = mysql_fetch_row(result);
				if (row == nullptr)
				{
					errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
					m_log->ToFile(ecLogType::eError, errorDescription);
					m_sql->Disconnect();
					// при ошибке считаем что запись есть
					return true;
				}

				int countPhone = std::stoi(row[0]);
				mysql_free_result(result);
				m_sql->Disconnect();

				if (countPhone >= 1)
				{
					return false;	// если есть запись, значит повторный звонок
				}

				return (countPhone == 0 ? false : true);
			}
		}
	}
}

// есть ли уже такой номер в БД (виртуальный оператор)
bool Queue::IsExistCallVirtualOperator(ecQueueNumber _queue, const std::string &_phone)
{
	std::string errorDescription;
	// правильней проверять сначало разговор	
	const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
		+ "' and phone = '" + _phone + "'"
		+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"
		+ " and hash is NULL order by date_time desc limit 1";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();		
		// при ошибке считаем что запись есть
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		return true;
	}

	unsigned int countPhone = std::stoi(row[0]);
	mysql_free_result(result);
	m_sql->Disconnect();

	if (countPhone >= 1)
	{
		return true;
	}
	else
	{		
		// нет разговора проверяем повтрность
		const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
			+ "' and phone = '" + _phone + "'"
			+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'" //тут типа ок, но время не затрагивается последние 15 мин
			+ " and hash is NULL order by date_time desc limit 1";

		if (!m_sql->Request(query, errorDescription))
		{
			errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			m_sql->Disconnect();			
			// при ошибке считаем что запись есть
			return true;
		}

		// результат
		MYSQL_RES *result = mysql_store_result(m_sql->Get());
		if (result == nullptr)
		{
			errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
			m_log->ToFile(ecLogType::eError, errorDescription);
			m_sql->Disconnect();
			// при ошибке считаем что запись есть
			return true;
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == nullptr)
		{
			errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
			m_log->ToFile(ecLogType::eError, errorDescription);
			m_sql->Disconnect();
			// при ошибке считаем что запись есть
			return true;
		}

		int countPhone = std::stoi(row[0]);

		mysql_free_result(result);
		m_sql->Disconnect();

		if (countPhone >= 1)
		{			
			return false; // считаем как новый вызов!!!
		}
		else
		{
			// проверка на повторность, вдруг еще раз перезвонили после того как поговорили уже	
			const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
				+ "' and phone = '" + _phone + "'"
				+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"			
				+ " and hash is not NULL order by date_time desc limit 1";

			if (!m_sql->Request(query, errorDescription))
			{
				errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
				m_log->ToFile(ecLogType::eError, errorDescription);

				m_sql->Disconnect();				
				// при ошибке считаем что запись есть
				return true;
			}

			// результат
			MYSQL_RES *result = mysql_store_result(m_sql->Get());
			if (result == nullptr)
			{
				errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
				m_log->ToFile(ecLogType::eError, errorDescription);
				m_sql->Disconnect();
				// при ошибке считаем что запись есть
				return true;
			}

			MYSQL_ROW row = mysql_fetch_row(result);
			if (row == nullptr)
			{
				errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
				m_log->ToFile(ecLogType::eError, errorDescription);
				m_sql->Disconnect();
				// при ошибке считаем что запись есть
				return true;
			}

			int countPhone = std::stoi(row[0]);
			mysql_free_result(result);
			m_sql->Disconnect();

			if (countPhone >= 1)
			{
				return false;	// если есть запись, значит повторный звонок
			}

			return (countPhone == 0 ? false : true);
		}		
	}
}

int Queue::GetLastQueueCallId(const std::string &_phone, const std::string &_call_id)
{
	std::string errorDescription;
	const std::string query = "select id from queue where phone = '" + _phone + "'" 
							+ " and id_ivr ='"+_call_id+"'";							

	if (!m_sql->Request(query))
	{
		m_sql->Disconnect();		
		return -1;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return -1;
	}

	int id = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return id;
}

// id записи по БД о звонке(виртуальный оператор)
int Queue::GetLastQueueVirtualOperatorCallId(const std::string &_phone)
{
	std::string errorDescription;
	const std::string query = "select id from queue_robot where phone = "
								+ _phone + " and date_time > '"
								+ GetCurrentStartDay() + "' order by date_time desc limit 1";

	if (!m_sql->Request(query))
	{
		m_sql->Disconnect();
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return -1;
	}

	int id = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return id;
}

bool Queue::GetCallsInBase(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription)
{
	_errorDescription = "";
	// найдем текущие номера которые будем трогать при обновлении
	std::string phoneTouch;

	for (const auto &list : _queueCalls)
	{
		if (phoneTouch.empty())
		{
			phoneTouch = "'" + list.phone + "'";
		}
		else
		{
			phoneTouch += ",'" + list.phone + "'"; // TODO проверить
		}
	}

	std::string query;
	if (!phoneTouch.empty()) 
	{
		query = "select id,phone,date_time from queue where date_time > '"
									+ GetCurrentStartDay()
									+ "' and answered = '1' and fail = '0' and hash is NULL and phone not in(" + phoneTouch + ")";
	} 
	else 
	{
		query = "select id,phone,date_time from queue where date_time > '"
									+ GetCurrentStartDay()
									+ "' and answered = '1' and fail = '0' and hash is NULL";
	}	
		
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

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	call.id = row[i];			break;
				case 1:	call.phone = row[i];		break;
				case 2:	call.date_time = row[i];	break;
			}			
		}

		call.hash = std::hash<std::string>()(call.phone + "_" + call.date_time);
		_vcalls.push_back(call);
		
		status = true;
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return status;
}

bool Queue::GetCallsInBase(CallsInBaseList &_vcalls, std::string &_errorDescription)
{
	const std::string query = "select id,phone,date_time from queue where date_time > '"
								+ GetCurrentStartDay() 
								+ "' and answered = '1' and fail = '0' and hash is NULL";

	
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

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
			case 0:	call.id = row[i];			break;
			case 1:	call.phone = row[i];		break;
			case 2:	call.date_time = row[i];	break;
			}			
		}

		call.hash = std::hash<std::string>()(call.phone + "_" + call.date_time);
		_vcalls.push_back(call);

		status = true;
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return status;
}

// получение записей из БД (виртуальный оператор)
bool Queue::GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription)
{
	_errorDescription.clear();
	// найдем текущие номера которые будем трогать при обновлении
	std::string phoneTouch;

	for (const auto &call : _queueCalls)
	{
		if (call.queue == ecQueueNumber::e5005) 
		{
			if (phoneTouch.empty())
			{
				phoneTouch = "'" + call.phone + "'";
			}
			else
			{
				phoneTouch += ",'" + call.phone + "'"; 
			}
		}		
	}

	std::string query;
	if (!phoneTouch.empty())
	{
		query = "select id,phone,date_time from queue_robot where date_time > '"
					+ GetCurrentStartDay()
					+ "' and hash is NULL and phone not in(" + phoneTouch + ")";
	}
	else
	{
		query = "select id,phone,date_time from queue_robot where date_time > '"
					+ GetCurrentStartDay()
					+ "' and hash is NULL";
	}

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

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	call.id = row[i];			break;
				case 1:	call.phone = row[i];		break;
				case 2:	call.date_time = row[i];	break;
			}
		}

		call.hash = std::hash<std::string>()(call.phone + "_" + call.date_time);
		_vcalls.push_back(call);

		status = true;
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return status;
}

// получение записей из БД (виртуальный оператор)
bool Queue::GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, std::string &_errorDescription)
{
	const std::string query = "select id,phone,date_time from queue_robot where date_time > '"
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

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	call.id = row[i];			break;
				case 1:	call.phone = row[i];		break;
				case 2:	call.date_time = row[i];	break;
			}			
		}

		call.hash = std::hash<std::string>()(call.phone + "_" + call.date_time);
		_vcalls.push_back(call);

		status = true;
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return status;
}

bool Queue::IsExistAnyAnsweredCall()
{
	std::string errorDescription;
	const std::string query = "select count(id) from queue where date_time > '"
							  + GetCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";
	
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();		
		// ошибка считаем что есть запись
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// ошибка считаем что есть запись
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// ошибка считаем что есть запись
		return true;
	}

	bool exist;
	(std::stoi(row[0]) == 0 ? exist = false : exist = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return exist;
}

void Queue::UpdateAllAnyAnsweredCalls()
{	
	std::string error;

	CallsInBaseList callsList;
	if (!GetCallsInBase(callsList, error)) 
	{
		printf("%s", error.c_str());
		return;
	}	

	// обновляем
	for (const auto &call : callsList)
	{
		const std::string query = "update queue set hash = '" + std::to_string(call.hash)
									+ "' where id ='" + call.id
									+ "' and phone ='" + call.phone
									+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, error);

			m_sql->Disconnect();						
			continue;
		}

	}

	m_sql->Disconnect();
}


