#include "ActiveSip.h"
#include "../utils/InternalFunction.h"
#include "../utils/custom_cast.h"
#include "../system/Constants.h"

#define PHONE_SIZE_INTERNAL 5

using namespace utils;
using namespace custom_cast;

static std::string SIP_COMMANDS_EXT1		= "ivr-5";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT2		= "Ring";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT3		= "Down";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT4		= "Outgoing";				// пропуск этой записи
static std::string SIP_COMMANDS_EXT5		= "FMPR";					// пропуск этой записи (эта запись на исходящий звонок)
static std::string SIP_COMMANDS_FND			= "func-apply-sipheaders|ext-queues";	// поиск по этой строке

static std::string SESSION_QUEUE_RESPONSE	= "asterisk -rx \"queue show %queue\"";
// static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -v \"" + SIP_COMMANDS_EXT1 + "\"" 
// 																							" | grep -v \"" + SIP_COMMANDS_EXT2 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT3 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT4 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT5 + "\"";
static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMANDS_FND + "\""
																							" | grep -v \"" + SIP_COMMANDS_EXT5 + "\"";


active_sip::ActiveSession::ActiveSession(SP_Queue &_queue)
	: IAsteriskData("ActiveSession",CONSTANTS::TIMEOUT::ACTIVE_SESSION)
	, m_queueSession(_queue)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::ACTIVE_SESSION))
{
}

active_sip::ActiveSession::~ActiveSession()
{
}

void active_sip::ActiveSession::Start()
{
	std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(SESSION_SIP_RESPONSE, error);
		};

	m_dispether.Start(func);	
}

void active_sip::ActiveSession::Stop()
{
	m_dispether.Stop();
	printf("ActiveSession stopped!\n");
}

void active_sip::ActiveSession::Parsing()
{
	// разбираем что же там по звонкам активным	
	CreateListActiveTalkCalls();	
		
	// найдем активных операторов в линии(смотрим текущие статичные очереди)
	CreateListActiveSessionOperators();	
	
	// что то есть нужно теперь в БД запихнуть
	if (IsExistListActiveTalkCalls())
	{
		UpdateActiveCurrentTalkCalls();
	}
	else
	{		
		// TODO СДЕЛАТЬ!!! тут что тоне так с алгоритмом!!! 
		
		// if (!IsExistListOperators())  // активных операторов в очередях не осталось
		// {
			
		// } 
		// else 
		// {
			if (!IsExistListActiveTalkCalls()) // не осталось активных звонков по базе астериска
			{
				// TODO тут перед выполнением такой опасной процедуры запросить еще раз 
				// напрямую из астера есть ли данные по активным звонкам
				if (!IxExistManualCheckCurrentTalk()) 
				{					
					m_queueSession->UpdateCallSuccess();
				}				
			}
		// }
	}		
}

// активные операторы в линии
void active_sip::ActiveSession::CreateListActiveSessionOperators()
{
	m_listOperators.clear(); 
	
	// без цикла очередей ecQueueNumber, просто берем нужные статичные очереди
	static const std::vector<ecQueueNumber> queueList =
	{
		ecQueueNumber::e5000,
		ecQueueNumber::e5050,
		ecQueueNumber::e5911,		
	};

	for (const auto &queue : queueList) 
	{
		std::string request = SESSION_QUEUE_RESPONSE;

		// заменяем %queue на номер очереди
		std::string repl = "%queue";
		size_t position = request.find(repl);
		request.replace(position, repl.length(), EnumToString<ecQueueNumber>(queue));

		std::string error;
		m_queue.DeleteRawAll(); // очистим все текущие данные 

		if (!m_queue.CreateData(request, error))
		{
			// TODO тут подумать что делать (пока след. очередь)
			continue;
		}

		// найдем активных операторов в линии
		CreateActiveOperators(queue);	
	}	
	
	// добавим\обновим очереди 
	InsertAndUpdateQueueNumberOperators();
}

void active_sip::ActiveSession::CreateListActiveTalkCalls()
{
	m_listCall.clear(); // очищаем текущий список с активными текущими разговорами по данным астериска

	std::string rawLines = IAsteriskData::GetRawLastData();
	if (rawLines.empty())
	{
		// TODO тут подумать, что делать!
		return;
	}

	if (!IsExistListOperators()) 
	{
		IAsteriskData::DeleteRawLastData(); // на всякий случай
		return;
	}	
	
	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))
	{
		for (const auto &sip : m_listOperators) 
		{
			ActiveTalkCall call;
			if (CreateActiveCall(line, sip.sipNumber, call))
			{
				// поищем id_ivr из сырых данных
				{
					std::istringstream ss(rawLines);
					std::string line;
					while (std::getline(ss, line))
					{					
						if (!FindActiveCallIvrID(line, call.phone_raw, call)) 
						{
							continue;
						}					
						
						//т.к нашли нет смысла дальше что то искать
						break;
					}
				}			
				m_listCall.push_back(call);
				break; // нет смысла дальше т.к. нашли нужные данные
			}
		}	
	}

	IAsteriskData::DeleteRawLastData(); // удаляем просмотренное
}

// найдем активных операторов в линии
void active_sip::ActiveSession::CreateActiveOperators(const ecQueueNumber _queue)
{	
	if (!m_queue.IsExistRaw()) 
	{
		// TODO нет данных надо убрать очередь из m_listOperators, подумать об этом!
		return;
	}
	
	std::istringstream ss(m_queue.GetRawLast());
	std::string line;

	while (std::getline(ss, line))
	{
		// заносить активные Callers не требуется
		if (line.find("Callers") != std::string::npos)
		{
			break;
		}

		// проверим есть ли активный sip
		if (line.find("Local/") != std::string::npos)
		{
			// найдем активный sip
			Operator sip;
			CreateOperator(line, sip, _queue);			

			// проверим есть ли уже такой sip чтобы добавить ему только очередь
			bool isExistOperator = false;
			for (auto &listOperators : m_listOperators) 
			{
				if (sip.sipNumber == listOperators.sipNumber) 
				{
					isExistOperator = true;
					listOperators.queueList.push_back(_queue); // т.к. находим именно эту очередь
					break;
				}
			}		

			if (!isExistOperator)
			{
				m_listOperators.push_back(sip);
			}
		}
	}
	
}

void active_sip::ActiveSession::CreateOperator(const std::string &_lines, Operator &_sip, ecQueueNumber _queue)
{
	_sip.sipNumber = FindSipNumber(_lines);
	_sip.queueList.push_back(_queue);
	_sip.isOnHold = FindOnHoldStatus(_lines);	

	// если onHold == true добавим номер с которым сейчас разговариет оператор
	if (_sip.isOnHold) 
	{
		AddPhoneOnHoldInOperator(_sip);
	}

}

// парсинг нахождения активного sip оператора
std::string active_sip::ActiveSession::FindSipNumber(const std::string &_lines)
{
	// 6 т.к. lenght("Local/) = 6	
	return _lines.substr(_lines.find_first_of("Local/") + 6, _lines.find_first_of("@") - _lines.find_first_of("Local/") - 6);
}

// парсинг нахождения статуса onHold
bool active_sip::ActiveSession::FindOnHoldStatus(const std::string &_lines)
{
	return ((_lines.find("On Hold") != std::string::npos) ? true : false);
}

// добавление\обновление номена очереди оператора в БД
void active_sip::ActiveSession::InsertAndUpdateQueueNumberOperators()
{
	// если пусто в активных сессиях операторов то нужно почистить БД
	if (!IsExistListOperators())
	{
		// очищаем номера очереди операторов
		if (IsExistOperatorsQueue())
		{
			ClearOperatorsQueue();			
			return;
		}
	}

	// проверим вдруг из очереди оператор убежал, тогда надо удалить sip из БД
	CheckOperatorsQueue();	


	// добавляем в БД
	for (const auto &sip : m_listOperators) 
	{
		for (size_t i = 0; i != sip.queueList.size(); ++i) 
		{
			if (!IsExistOperatorsQueue(sip.sipNumber, EnumToString<ecQueueNumber>(sip.queueList[i]))) 
			{
				//записи нет добавляем
				InsertOperatorsQueue(sip.sipNumber, EnumToString<ecQueueNumber>(sip.queueList[i]));
			}
		}
	}	
}

bool active_sip::ActiveSession::IsExistListOperators()
{
	return !m_listOperators.empty();
}

// есть ли данные в m_listOperators.onHold
bool active_sip::ActiveSession::IsExistListOperatorsOnHold()
{
	for (const auto &sip : m_listOperators)
	{
		if (sip.isOnHold) return true;
	}
		
	return false;
}

bool active_sip::ActiveSession::IsExistListActiveTalkCalls()
{
	return (!m_listCall.empty() ? true : false);
}

// существует ли хоть 1 запись в БД sip+очередь
bool active_sip::ActiveSession::IsExistOperatorsQueue()
{
	std::string errorDescription;
	const std::string query = "select count(id) from operators_queue";

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

	bool existOperatorsQueue;
	(std::stoi(row[0]) == 0 ? existOperatorsQueue = false : existOperatorsQueue = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existOperatorsQueue;
}

// существует ли хоть запись в БД sip+очередь
bool active_sip::ActiveSession::IsExistOperatorsQueue(const std::string &_sip, const std::string &_queue)
{
	const std::string query = "select count(id) from operators_queue where sip = '" + _sip + "' and queue = '" + _queue + "'";

	std::string errorDescription;
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
	std::stoi(row[0]) == 0 ? exist = false : exist = true;
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return exist;
}

// очистка таблицы operators_queue
void active_sip::ActiveSession::ClearOperatorsQueue()
{
	std::string error;
	const std::string query = "delete from operators_queue";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();
		return;
	}
	
	m_sql->Disconnect();
}

// проверка есть ли оператор еще в очереди
void active_sip::ActiveSession::CheckOperatorsQueue()
{
	OperatorList listActiveOperatorsBase;

	std::string errorDescription;
	if (!GetActiveQueueOperators(listActiveOperatorsBase, errorDescription))
	{
		errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		return;
	}

	// проверим совпадают ли данные с данными по БД
	for (const auto &curr_list : listActiveOperatorsBase)
	{
		bool isExistSip = true;		// считаем что sip по умолчанию существует
		bool isExistQueue = true;	// считаем что queue по умолчанию существует

		for (const auto &memory_list : m_listOperators)
		{
			if (curr_list.sipNumber == memory_list.sipNumber)
			{
				isExistSip = true;

				// проверим есть ли такая очередь
				for (size_t i = 0; i < memory_list.queueList.size(); ++i)
				{
					if (curr_list.queueList[0] == memory_list.queueList[i])
					{
						isExistQueue = true;
						break;
					}
					else
					{
						isExistQueue = false;
					}
				}

				break;
			}
			else
			{
				// нет sip, надо удалить из БД
				isExistSip = false;
			}
		}

		// что именно будем удалять из БД
		if (isExistSip)
		{
			if (!isExistQueue)
			{
				// удаляем sip + очередь конкретную
				DeleteOperatorsQueue(curr_list.sipNumber, EnumToString<ecQueueNumber>(curr_list.queueList[0]));
			}
		}
		else
		{
			// удаляем весь sip
			DeleteOperatorsQueue(curr_list.sipNumber);
		}
	}
}

bool active_sip::ActiveSession::GetActiveQueueOperators(OperatorList &_activeList, std::string &_errorDescription)
{
	// найдем данные по БД
	_errorDescription = "";
	const std::string query = "select sip,queue from operators_queue";
	
	if (!m_sql->Request(query, _errorDescription))
	{		
		_errorDescription += METHOD_NAME + StringFormat("query \t%s", query.c_str());
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
		Operator activeOperator;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
			case 0:	activeOperator.sipNumber = row[i]; break;
			case 1: activeOperator.queueList.push_back(StringToEnum<ecQueueNumber>(row[i])); break;
			}
		}
		_activeList.push_back(activeOperator);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

// удаление очереди оператора из БД таблицы operators_queue
void active_sip::ActiveSession::DeleteOperatorsQueue(const std::string &_sip, const std::string &_queue)
{
	const std::string query = "delete from operators_queue where sip = '" + _sip + "' and queue = '" + _queue + "'";

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

// удаление очереди оператора из БД таблицы operators_queue весь sip
void active_sip::ActiveSession::DeleteOperatorsQueue(const std::string &_sip)
{
	const std::string query = "delete from operators_queue where sip = '" + _sip + "'";

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

// добавление очереди оператору в БД таблицы operators_queue
void active_sip::ActiveSession::InsertOperatorsQueue(const std::string &_sip, const std::string &_queue)
{
	const std::string query = "insert into operators_queue (sip,queue) values ('"
								+ _sip + "','"
								+ _queue + "')";

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

bool active_sip::ActiveSession::CreateActiveCall(const std::string &_lines, const std::string &_sipNumber, ActiveTalkCall &_caller)
{	
	/*
		ЧТОБЫ НЕ ЗАБЫТЬ КОГДА ДОБЕРУСЬ ДО СЮДА!!
		разбор идет по 2ой строке!!
	SIP/Dru_220000-0002b661!ext-queues!5000!54!Up!Queue!5000,tcc,,,50,,,,,!79275052333!!!3!276!0870397a-eedf-4c39-9e58-556f4e838e0e!1758655112.320303
	Local/64197@from-queue-00011669;2!macro-dial-one!s!55!Up!Dial!sip/64197,,trM(auto-blkvm)Ib(func-apply-sipheaders^s^1)!79275052333!!!3!234!2f3f7fa2-a55f-45d4-8ffa-77abdd22cba4!1758655155.320311
	[root@srvasterisk ~]# clear && asterisk -rx "core show channels concise" | grep -v "ivr-5" | grep -v "Ring" | grep -v "Down" | grep -v "FMPR" | grep -v "Outgoing"

	*/
	
	if (_lines.find("Local/" + _sipNumber) == std::string::npos)
	{
		// нет нужной строки на выход
		return false;
	}

	if (_lines.find("Ring") != std::string::npos ||
		_lines.find("Down") != std::string::npos ||
		_lines.find("Outgoing") != std::string::npos)
	{
		// нет нужной строки на выход
		return false;
	}

	std::vector<std::string> lines;
	
	std::istringstream iss(_lines);
    std::string token;
    while (std::getline(iss, token, '!')) 
	{
		if (token.empty())
		{
			continue;
		}		 

		lines.push_back(token);
	}
     
	if ((lines.empty()) || (lines.size() < 11)) 
	{
		return false;
	}	

	_caller.sip = _sipNumber;
	

	// for (int i=0; i<8; ++i) 
	// {
	// 	printf("lines[%u] - %s\n", i, lines[i].c_str());
	// } 


	try 
	{
		lines.at(7).size() != PHONE_SIZE_INTERNAL ? _caller.phone = PhoneParsing(lines.at(7))
							    				  : _caller.phone = PhoneParsingInternal(lines.at(7)); // считаем что звонок внутренний для техподдержки ИК  
		
		_caller.phone_raw = lines.at(7);
    }
    catch (const std::out_of_range& e) 
	{
        auto msgErr = StringFormat("%s\t lines\t %s\t what=%s ", METHOD_NAME, _lines.c_str(), e.what());
		m_log->ToFile(ecLogType::eError, msgErr);        
		
		return false;
    }	
	
	try 
	{
		_caller.talkTime = lines.at(9);
    }
    catch (const std::out_of_range& e) 
	{
        auto msgErr = StringFormat("%s\t lines\t %s\t what=%s ", METHOD_NAME, _lines.c_str(), e.what());
		m_log->ToFile(ecLogType::eError, msgErr);        
		
		return false;
    }

	try 
	{
		_caller.callID = lines.at(11);
    }
    catch (const std::out_of_range& e) 
	{
        auto msgErr = StringFormat("%s\t lines\t %s\t what=%s ", METHOD_NAME, _lines.c_str(), e.what());
		m_log->ToFile(ecLogType::eError, msgErr);        
		
		return false;
    }

	return CheckActiveCall(_caller);	
}

bool active_sip::ActiveSession::FindActiveCallIvrID(const std::string &_lines, const std::string &_phone, ActiveTalkCall &_caller)
{
	if (_lines.find(_phone) == std::string::npos)
	{
		// нет нужной строки на выход
		return false;
	}
	
	if (_lines.find("Local/") != std::string::npos) 
	{
		if (_lines.find(_phone) == std::string::npos)
		{
			// нет нужной строки на выход
			return false;
		}		
	}

	std::vector<std::string> lines;
	
	std::istringstream iss(_lines);
    std::string token;
    while (std::getline(iss, token, '!')) 
	{
		if (token.empty())
		{
			continue;
		}		 

		lines.push_back(token);
	}
     
	if ((lines.empty()) || (lines.size() < 11)) 
	{
		return false;
	}	
		
	try 
	{
		// чтобы не звхватить id разговора
		if (_caller.callID == lines.at(11)) 
		{
			return false;	
		}

		_caller.ivr_callID = lines.at(11);	
    }
    catch (const std::out_of_range& e) 
	{
        auto msgErr = StringFormat("%s\t lines\t %s\t what=%s ", METHOD_NAME, _lines.c_str(), e.what());
		m_log->ToFile(ecLogType::eError, msgErr);        
		
		return false;
    }

	return true;
}

bool active_sip::ActiveSession::CheckActiveCall(const ActiveTalkCall &_caller)
{
	return  !_caller.phone.empty() 	&&
			 !_caller.sip.empty()	&&
			 !_caller.talkTime.empty() &&
			 !_caller.callID.empty();
}

// обновление текущих звонков операторов
void active_sip::ActiveSession::UpdateActiveCurrentTalkCalls()
{
	// обновление данных таблицы queue о том с кем сейчас разговаривает оператор
	UpdateTalkCallOperator();
	
	// OnHold проверка вдруг оператор разговаривает и поставил на удержание звонок
	UpdateOnHoldStatusOperator();	
}

// обновление данных таблицы queue о том с кем сейчас разговаривает оператор
void active_sip::ActiveSession::UpdateTalkCallOperator()
{
	if (!IsExistListActiveTalkCalls()) 
	{
		return;
	}

	for (const auto &call : m_listCall)
	{
		// проверим есть ли такой номер
		if (!IsExistTalkCallOperator(call.phone)) 
		{
			continue;
		}		
		 
		std::string id = std::to_string(GetLastTalkCallOperatorID(call.phone));
		if (id.find("-1") != std::string::npos)
		{
			continue;
		}

		const std::string query = "update queue set sip = '" + call.sip 
															 + "', talk_time = '" + GetTalkTime(call.talkTime)
															 +"', call_id = '" 	+ call.callID
															 + "', id_ivr = '"+ call.ivr_callID
															 + "', answered ='1' where phone = '" + call.phone + 
															 "' and id ='" + id + "'";

		std::string error;
		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
			m_log->ToFile(ecLogType::eError, error);

			m_sql->Disconnect();
			
			continue;
		}

		m_sql->Disconnect();		
	}	
}

bool active_sip::ActiveSession::IsExistTalkCallOperator(const std::string &_phone)
{
	const std::string query = "select count(phone) from queue where phone = '" + _phone
								+ "' and date_time > '" + GetCurrentStartDay()
								+ "' order by date_time desc limit 1";

	std::string errorDescription;
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

	bool existQueueSip;
	std::stoi(row[0]) == 0 ? existQueueSip = false : existQueueSip = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existQueueSip;
}

// получение последнего ID актуального разговора текущего оператора в таблице queue
int active_sip::ActiveSession::GetLastTalkCallOperatorID(const std::string &_phone)
{
	const std::string query = "select id from queue where phone = "
							+ _phone + " and date_time > '"
							+ GetCurrentStartDay() + "' order by date_time desc limit 1";

	std::string errorDescription;
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		
		// ошибка считаем что нет записи
		return -1;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// ошибка считаем что нет записи
		return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		// ошибка считаем что нет записи
		return -1;
	}

	int count = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return count;
}

// добавление номера телефона который на onHold сейчас
void active_sip::ActiveSession::AddPhoneOnHoldInOperator(Operator &_sip)
{
	for (const auto &call : m_listCall) 
	{
		if (call.sip == _sip.sipNumber) 
		{
			_sip.phoneOnHold = call.phone;
			break;
		}
	}
}

// обновление статуса onHold
void active_sip::ActiveSession::UpdateOnHoldStatusOperator()
{
	OnHoldList onHoldList;

	std::string errorDescription;
	// найдем операторов которые по БД числяться в onHold
	if (!GetActiveOnHold(onHoldList, errorDescription))
	{
		errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		return;
	}	

	if (!IsExistListOperators()) 
	{
		// нету активных операторов, значит нужно отключить по БД все onHold 
		if (!onHoldList.empty()) 
		{
			DisableOnHold(onHoldList);
		}				
		return;
	}	

	// основная проверка onHold
	CheckOnHold(onHoldList);
}

// получение всех onHold стаутсов которые есть в БД
bool active_sip::ActiveSession::GetActiveOnHold(OnHoldList &_onHoldList, std::string &_errorDescription)
{
	_errorDescription.clear();
	_onHoldList.clear();
	
	// найдем все OnHold которые не завершенные 
	const std::string query = "select id,sip,phone from operators_ohhold where date_time_stop is NULL order by date_time_start DESC";
	

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
		OnHold onHold;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
			case 0:	onHold.id = std::atoi(row[i]);	break;	// id
			case 1:	onHold.sip = row[i];			break;	// sip
			case 2:	onHold.phone = row[i];			break;	// phone
			}
		}
		
		if (onHold.check())
		{
			_onHoldList.push_back(onHold);
		}		
	}

	mysql_free_result(result);	
	m_sql->Disconnect();

	return true;
}

// очистка всех операторов которые в статусе onHold по БД
void active_sip::ActiveSession::DisableOnHold(const OnHoldList &_onHoldList)
{
	for (const auto &hold : _onHoldList) 
	{
		std::string errorDescription;
		if (!DisableHold(hold, errorDescription))
		{
			errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			continue;
		}
	}
}

// отключение onHold в БД
bool active_sip::ActiveSession::DisableHold(const OnHold &_hold, std::string &_errorDescription)
{
	_errorDescription.clear();
	const std::string query = "update operators_ohhold set date_time_stop = '" + GetCurrentDateTime() 
							+ "' where id = '" + std::to_string(_hold.id)
							+ "' and sip = '" + _hold.sip + "'";

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

	return true;
}

// добавление нового onHold в БД
bool active_sip::ActiveSession::AddHold(const Operator &_sip, std::string &_errorDescription)
{
	const std::string query = "insert into operators_ohhold (sip,phone) values ('" + _sip.sipNumber + "','" + _sip.phoneOnHold + "')";
	
	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}
	m_sql->Disconnect();

	return true;
}

// Основная проверка отключение\добавление onHold 
void active_sip::ActiveSession::CheckOnHold(OnHoldList &_onHoldList)
{
	// надо сначало существующие проверить
	bool needNewHoldList = false; // флаг того что нужно пересчитать HoldList
	std::string errorDescription;

	for (const auto &hold : _onHoldList) 
	{
		bool existHold = false; // по умолчанию считаем что нет активного активного hold
		for (const auto &sip : m_listOperators)
		{
			if (hold.sip == sip.sipNumber) 
			{
				if (sip.isOnHold) 
				{
					existHold = true;
					break;
				}				
			}
		}

		if (!existHold) 
		{			
			if (!DisableHold(hold, errorDescription))
			{
				errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
				m_log->ToFile(ecLogType::eError, errorDescription);

				return;
			}

			needNewHoldList = true;
		}
	}
	
	// надо ли пересчитать HoldList
	if (needNewHoldList) 
	{
		if (!GetActiveOnHold(_onHoldList, errorDescription))
		{
			errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);

			return;
		}
	}	
	
	// проверим новые теперь
	for (const auto &sip : m_listOperators) 
	{
		bool newHold = false; // новый hold
		if (_onHoldList.empty())  // т.к. пусто в БД, то нечего проверять
		{
			if (sip.isOnHold) 
			{
				newHold = true;
			}			
		}
		else 
		{
			for (const auto &hold : _onHoldList)
			{
				// проверим если такой hold в БД
				if (sip.isOnHold)
				{
					// по умолчанию считаем что он новый 
					newHold = true;

					if (sip.sipNumber == hold.sip)
					{
						newHold = false;
						break;
					}
				}
			}
		}

		// добавим новый hold
		if (newHold)
		{
			std::string errorDescription;

			if (sip.phoneOnHold != "null")
			{
				if (!AddHold(sip, errorDescription))
				{
					errorDescription = StringFormat("%s\t%s", METHOD_NAME, errorDescription.c_str());
					m_log->ToFile(ecLogType::eError, errorDescription);
				}
			}
		}
	}
}

bool active_sip::ActiveSession::IxExistManualCheckCurrentTalk()
{
    std::string errorDescription; 
	m_rawDataTalkCall.DeleteRawAll();

	if (!m_rawDataTalkCall.CreateData(SESSION_SIP_RESPONSE,errorDescription)) 
	{
		// TODO тут считаем что данные есть в случае какой либо ошибки!
		if (errorDescription.find("empty") == std::string::npos) 
		{
			errorDescription = StringFormat("%s\t%s",METHOD_NAME,errorDescription.c_str());
			m_log->ToFile(ecLogType::eError, errorDescription);
		}	
		
		return false;
	}

	std::string rawLines = m_rawDataTalkCall.GetRawFirst();
	
	return !rawLines.empty();	
}
