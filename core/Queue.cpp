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

static std::string _extractedPart(const std::string	&_str) 
{
	size_t pos = _str.find('.');	

	// Если точка найдена, извлекаем подстроку   
    if (pos != std::string::npos) 
	{
        return 	_str.substr(0, pos + 1); // +1 для включения точки
    } 
	else 
	{
        return _str; 	// Если точки нет, берем всю строку
    }
}


Queue::Queue()
	: IAsteriskData("Queue",CONSTANTS::TIMEOUT::QUEUE)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::QUEUE))

#ifdef CREATE_LOG_DEBUG
	, m_logRaw(std::make_shared<Log>(static_cast<string>("queue_raw.log")))
#endif		
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

#ifdef CREATE_LOG_DEBUG
	static int id = 0;
#endif

	while (std::getline(ss, line))
	{
		QueueCalls queueCaller;

		if (CreateQueueCallers(line, queueCaller))
		{
			m_listQueue.push_back(queueCaller);
			
#ifdef CREATE_LOG_DEBUG
			m_logRaw->ToFile(ecLogType::eInfo, StringFormat("[%u]  %s", id, line.c_str()));
#endif

		}
	}

#ifdef CREATE_LOG_DEBUG
	++id;
	
	std::string phoneList;
	for (const auto &it : m_listQueue) 
	{
		if (phoneList.empty()) 
		{
			phoneList = it.phone;
		} else 
		{
			phoneList += "," + it.phone;
		}
	}

	m_log->ToFile(ecLogType::eDebug, StringFormat("m_listQueue phone(%d) = %s", m_listQueue.size(), phoneList.c_str()));
#endif

	return IsExistQueueCalls();	
}

bool Queue::CreateQueueCallers(const std::string &_lines, QueueCalls &_queueCaller)
{
	std::vector<std::string> lines;
	std::string errorDescription;
	if (!ParsingAsteriskRawDataRequest(lines,_lines,errorDescription)) 
	{
		return false;
	}

	_queueCaller.queue = StringToEnum<ecQueueNumber>(lines[2]);										  // номер очереди
	_queueCaller.state = StringToEnum<ecAsteriskState>(lines[4]);									  // текущее состояние канала (Up, Ring, Down и т.п.)
	_queueCaller.application = StringToEnum<ecAsteriskApp>(lines[5]);								  // текущее приложение(Dial, Playback, …)
	_queueCaller.phone = (_queueCaller.queue != ecQueueNumber::e5911) ? utils::PhoneParsing(lines[7]) // текущий номер телефона который в очереди сейчас
																	  : utils::PhoneParsingInternal(lines[7]);
	_queueCaller.waiting = static_cast<uint16_t>(std::stoi(lines[11])); // время ожидания
	_queueCaller.call_id = lines[13];									// id звонка (ivr)

	
	// не прошел по какой то причине звонок
	if (!CheckCallers(_queueCaller))
	{
		errorDescription = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

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

	// находим и обновляем данные когда у нас звонок из IVR попал в очередь\на виртаульного оператора
	UpdateCallIvr(_callList);

	// находим и обновляем данные когда разговор успешно состоялся 
	UpdateCallSuccess(_callList);

	// находим и обновляем данные если звонок был в очереди, но не дождался ответа от оператора
	UpdateCallFail(_callList);
}

void Queue::InsertCall(const QueueCalls &_call)
{
	std::string errorDescription;
	bool errorConnectSQL = false;

	if (IsExistCall(_call, errorConnectSQL)) 
	{
		// возникла ошибка при запросе к БД выходим
		if (errorConnectSQL) 
		{
			return;
		}
		
		// номер существует, обновляем данные
		int id = GetLastQueueCallId(_call.phone, _call.call_id);
		
		if (id == -1) 
		{

#ifdef CREATE_LOG_DEBUG
			errorDescription = StringFormat("%s\tnot found phone = %s and call_id = %s\t", METHOD_NAME,
											_call.phone.c_str(),
											_call.call_id.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);
#endif
			return; 
		}

		if (!UpdateCall(id, _call, errorDescription))
		{
			m_log->ToFile(ecLogType::eError, errorDescription);
			return;
		}	
	}
	else 
	{		
		// возникла ошибка при запросе к БД выходим
		if (errorConnectSQL) 
		{
			return;
		}

		// проверим на всякий случай нет ли уже такого значения id_ivr, т.к. оно уникальное!
		if (IsExixtCall_CallID(_call)) 
		{
			return;
		}
		
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

#ifdef CREATE_LOG_DEBUG
		m_log->ToFile(ecLogType::eInfo, query);
#endif

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
	const std::string query = "update queue set waiting_time = '" 	+  GetTalkTime(_call.waiting) 								
																	+ "' where id ='" + std::to_string(_id) + "'";
	
	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription = StringFormat("%s: %s\tquery \t%s", METHOD_NAME, _errorDescription.c_str(), query.c_str());
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
			phoneDoNotTouch += ",'" + list.phone + "'";	
		}
	}	
	
	// обновляем данные	
	const std::string query = std::string("update queue set fail = '1' where answered = '0'") +
                              std::string(" and sip = '-1' and phone not in (") + phoneDoNotTouch + ")";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();		
		return;
	}
	m_sql->Disconnect();

	
	// проверим есть ли ошибочные fail(я хз как они появляются)
	FindErroneousError();
}

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

bool Queue::IsExistCall(const QueueCalls &_call, bool &_errorConnectSQL)
{
	// есть ли активный разговор
	if (IsExixtCall_ActiveTalkCall(_call, _errorConnectSQL))
	{
		return true;
	}

	// вдруг в очереди сейчас находится звонок
	if (IsExixtCall_ActiveTalkQueue(_call, _errorConnectSQL))
	{
		return true;
	}

	// нет разговора проверяем повтрность
	if (IsExixtCall_ActiveTalkRepeat(_call, _errorConnectSQL))
	{
		return true;
	}

	// проверка на повторность, вдруг еще раз перезвонили после того как поговорили уже
	if (IsExixtCall_ActiveTalkRepeatTwo(_call, _errorConnectSQL))
	{
		return true;
	}

	// вдруг из лизы пришел	
	if (IsExixtCall_ReturnToLisa(_call, _errorConnectSQL))
	{
		return true;
	}

	return false;
}

bool Queue::IsExixtCall_ActiveTalkCall(const QueueCalls &_call, bool &_errorConnectSQL)
{
	std::string errorDescription;
	
	// проверяем разговор
	const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString<ecQueueNumber>(_call.queue) 
																					  + "' and phone = '" + _call.phone + "'" 
																					  + " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL " 
																					  + " and id_ivr = '" + _call.call_id + "'";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
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
		_errorConnectSQL = true;
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		return true;
	}

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		
		mysql_free_result(result);
		m_sql->Disconnect();
		
		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return countPhone;	
}

bool Queue::IsExixtCall_ActiveTalkQueue(const QueueCalls &_call, bool &_errorConnectSQL)
{
	std::string errorDescription;
	const std::string query = "select count(phone) from queue where phone = '" 	+ _call.phone + "'" 
																				+ " and answered ='0' and fail='0' and sip='-1' and hash is NULL" 
																				+ " and id_ivr = '" + _call.call_id + "'";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
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
		_errorConnectSQL = true;
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		return true;
	}

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;

		mysql_free_result(result);
		m_sql->Disconnect();

		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return countPhone;	
}

bool Queue::IsExixtCall_ActiveTalkRepeat(const QueueCalls &_call, bool &_errorConnectSQL)
{
	std::string errorDescription;
	const std::string query = "select count(phone) from queue where phone = '" 	+ _call.phone + "'"			
																				+ " and answered ='0' and fail='1' and sip = '-1' and hash is NULL"
																				+ " and id_ivr = '"+_call.call_id+"'";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
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
		_errorConnectSQL = true;
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		return true;
	}

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;

		mysql_free_result(result);
		m_sql->Disconnect();

		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return countPhone;	
}

bool Queue::IsExixtCall_ActiveTalkRepeatTwo(const QueueCalls &_call, bool &_errorConnectSQL)
{
	std::string errorDescription;
	const std::string query = "select count(phone) from queue where phone = '" 	+ _call.phone + "'"				
																				+ " and answered = '1' and fail = '0' and sip <>'-1'"
																				+ " and hash is not NULL"
																				+ " and id_ivr = '" + _call.call_id + "'";	
	
	
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();					
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
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
		_errorConnectSQL = true;
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		return true;
	}

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть
		_errorConnectSQL = true;
		
		mysql_free_result(result);
		m_sql->Disconnect();

		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return countPhone;		
}

//  повторный звонок по кругу гоняется пришел из лизы
bool Queue::IsExixtCall_ReturnToLisa(const QueueCalls &_call, bool &_errorConnectSQL)
{
	std::string errorDescription;	
	std::string query = "select count(phone) from queue where phone = '" 	+ _call.phone + "'"																				
																			+ " and (call_id = '" + _call.call_id + "' or id_ivr = '"+_call.call_id+"')" ;	
	
	
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

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть		
		
		mysql_free_result(result);
		m_sql->Disconnect();

		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

	if (countPhone > 0) 
	{

#ifdef CREATE_LOG_DEBUG
		errorDescription = StringFormat("%s\t phone = %s, call_id = %s, state = %s IS %s", METHOD_NAME,
										_call.phone.c_str(),
										_call.call_id.c_str(),
										EnumToString<ecAsteriskState>(_call.state).c_str(),
										(countPhone) ? "EXIST" : "NOT EXIST");
		m_log->ToFile(ecLogType::eInfo, errorDescription);
#endif

		return true;
	}	

	return countPhone;		
}

bool Queue::IsExixtCall_CallID(const QueueCalls &_call)
{
	std::string errorDescription;

	// если не нашли значит поищем более тонкую настройку, т.е. будем искать только первуючасть id все что до "."		
	std::string callLike = _extractedPart(_call.call_id);	
		
	std::string query = "select count(phone) from queue where phone = '"+ _call.phone + "'"																				
															+ " and (call_id like '" + callLike + "%' or id_ivr = '"+callLike+"%')" ;	
	
	
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

	int countPhone = 0;
	try
	{
		countPhone = std::stoi(row[0]);	
	}
	catch(const std::exception& e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);		
		// при ошибке считаем что запись есть		
		
		mysql_free_result(result);
		m_sql->Disconnect();

		return true;
	}		
	
	mysql_free_result(result);
	m_sql->Disconnect();

#ifdef CREATE_LOG_DEBUG
	errorDescription = StringFormat("%s\t !!LIKE!! phone = %s, call_id = %s, state = %s IS %s", METHOD_NAME,
									_call.phone.c_str(),
									_call.call_id.c_str(),
									EnumToString<ecAsteriskState>(_call.state).c_str(),
									(countPhone) ? "EXIST" : "NOT EXIST");
	m_log->ToFile(ecLogType::eInfo, errorDescription);
#endif

	return countPhone;	
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
							+ " and (call_id = '" + _call_id + "' or id_ivr = '"+ _call_id +"')"; 							

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
			phoneTouch += ",'" + list.phone + "'"; // TODO передалать на внешнюю функцию
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


void Queue::FindErroneousError()
{
	if (!IsExistErroneousError())
	{
		return;
	}

	UpdateErroneousError();
}	


bool Queue::IsExistErroneousError()
{
	std::string errorDescription;
	const std::string query = "select count(id) from queue where date_time > '"
							  + GetCurrentStartDay() + "' and answered = '1' and fail = '1' and sip <> '-1' and hash is NULL";
	
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();	
		return false;
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
		// ошибка считаем что есть запись
		return false;
	}

	bool existField = false;
	try
	{
		existField = (std::stoi(row[0]) != 0);
	}
	catch (const std::exception &e)
	{
		errorDescription = StringFormat("!!exception!! %s\t%s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);
		mysql_free_result(result); // Освобождаем результат
		m_sql->Disconnect();
		return false;
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return existField;	
}

void Queue::UpdateErroneousError()
{
	std::string errorDescription;
	const std::string query = "update queue set fail = '0' where date_time > '"
							  + GetCurrentStartDay() + "' and answered = '1' and fail = '1' and hash is NULL";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);	
	}

	m_sql->Disconnect();
}

