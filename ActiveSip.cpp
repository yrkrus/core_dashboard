#include "ActiveSip.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "utils.h"

using namespace utils;

active_sip::ActiveSession::ActiveSession(SP_Queue &_queue)
	:IAsteriskData(CONSTANTS::TIMEOUT::ACTIVE_SESSION)
	, m_queueSession(_queue)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::ACTIVE_SESSION)
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
}

void active_sip::ActiveSession::Parsing()
{
	// разбираем что же там по звонкам активным
	m_listCall.clear(); // TODO тут пока обнуление, потом подумать чтобы не делать так а работать с памятью !!
	CreateListActiveSessionCalls();
	
	m_listOperators.clear(); // TODO тут пока обнуление, потом подумать, чтобы так не делать а работать уже с памятью!!!	
	// найдем активных операторов в линии(смотрим текущие статичные очереди)
	CreateListActiveSessionOperators();
	
	
	// что то есть нужно теперь в БД запихнуть
	if (IsExistListCalls())
	{
		UpdateActiveSessionCalls();
	}
	else
	{
		if (!IsExistListOperators())
		{
			// вдруг никого не осталось, тогда еще раз проверим очнереди
			m_queueSession->UpdateCallSuccess();
		}
	}		
}

// активные операторы в линии
void active_sip::ActiveSession::CreateListActiveSessionOperators()
{
	// INFO: прежде чем сюда попасть полностью очищается m_listOperators!
	
	// без цикла очередей EQueueNumber, просто берем нужные статичные очереди
	static const std::vector<EQueueNumber> queueList =
	{
		EQueueNumber::e5000,
		EQueueNumber::e5050,		
	};

	for (const auto &queue : queueList) 
	{
		std::string request = SESSION_QUEUE_RESPONSE;

		// заменяем %queue на номер очереди
		std::string repl = "%queue";
		size_t position = request.find(repl);
		request.replace(position, repl.length(), EnumToString<EQueueNumber>(queue));

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

void active_sip::ActiveSession::CreateListActiveSessionCalls()
{
	std::string rawLines = GetRawLastData();
	if (rawLines.empty())
	{
		// TODO тут подумать, что делать!
		return;
	}

	if (!IsExistListOperators()) 
	{
		DeleteRawLastData(); // на всякий случай
		return;
	}	
	
	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))
	{
		for (const auto &sip : m_listOperators) 
		{
			ActiveCall call;
			if (CreateActiveCall(line, sip.sipNumber, call))
			{
				m_listCall.push_back(call);
				break; // нет смысла дальше т.к. нашли нужные данные
			}
		}	
	}

	DeleteRawLastData(); // удаляем просмотренное
}

// найдем активных операторов в линии
void active_sip::ActiveSession::CreateActiveOperators(const EQueueNumber _queue)
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

void active_sip::ActiveSession::CreateOperator(const std::string &_lines, Operator &_sip, EQueueNumber _queue)
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
			if (!IsExistOperatorsQueue(sip.sipNumber, EnumToString<EQueueNumber>(sip.queueList[i]))) 
			{
				//записи нет добавляем
				InsertOperatorsQueue(sip.sipNumber, EnumToString<EQueueNumber>(sip.queueList[i]));
			}
		}
	}	
}

bool active_sip::ActiveSession::IsExistListOperators()
{
	return (!m_listOperators.empty() ? true : false);
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

bool active_sip::ActiveSession::IsExistListCalls()
{
	return (!m_listCall.empty() ? true : false);
}

// существует ли хоть 1 запись в БД sip+очередь
bool active_sip::ActiveSession::IsExistOperatorsQueue()
{
	std::string error;
	const std::string query = "select count(id) from operators_queue";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);
		
		m_sql->Disconnect();
		// ошибка считаем что есть запись
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

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

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		// ошибка считаем что есть запись
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		return;
	}
	
	m_sql->Disconnect();
}

// проверка есть ли оператор еще в очереди
void active_sip::ActiveSession::CheckOperatorsQueue()
{
	OperatorList listActiveOperatorsBase;

	std::string error;
	if (!GetActiveQueueOperators(listActiveOperatorsBase, error))
	{
		error += METHOD_NAME;
		m_log.ToFile(ELogType::Error, error);

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
				DeleteOperatorsQueue(curr_list.sipNumber, EnumToString<EQueueNumber>(curr_list.queueList[0]));
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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();
		return false;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Operator activeOperator;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
			case 0:	activeOperator.sipNumber = row[i]; break;
			case 1: activeOperator.queueList.push_back(StringToEnum<EQueueNumber>(row[i])); break;
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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);
		
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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return;
	}

	m_sql->Disconnect();
}

bool active_sip::ActiveSession::CreateActiveCall(const std::string &_lines, const std::string &_sipNumber, ActiveCall &_caller)
{	
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

		if (_lines[i] != '!') // ищем разделить (разделить !)
		{
			current_str += _lines[i];
			isNewLine = false;
		}
		else
		{
			isNewLine = true;
		}
	}

	if ((lines.empty()) || (lines.size() < 10)) 
	{
		return false;
	}

	_caller.sip = _sipNumber;
	_caller.phone = PhoneParsing(lines[7]);
	_caller.talkTime = lines[9];	

	return CheckActiveCall(_caller);	
}

bool active_sip::ActiveSession::CheckActiveCall(const ActiveCall &_caller)
{
	return !((_caller.phone == "null")	&&
			 (_caller.sip == "null")	&&
			 (_caller.talkTime == "null"));
}

// обновление текущих звонков операторов
void active_sip::ActiveSession::UpdateActiveSessionCalls()
{
	// обновление данных таблицы queue о том с кем сейчас разговаривает оператор
	UpdateTalkCallOperator();
	
	// OnHold проверка вдруг оператор разговаривает и поставил на удержание звонок
	UpdateOnHoldStatusOperator();	
}

// обновление данных таблицы queue о том с кем сейчас разговаривает оператор
void active_sip::ActiveSession::UpdateTalkCallOperator()
{
	if (!IsExistListCalls()) 
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

		const std::string query = "update queue set sip = '"
									+ call.sip + "', talk_time = '"
									+ getTalkTime(call.talkTime)
									+ "', answered ='1' where phone = '"
									+ call.phone + "' and id ='"
									+ id + "'";

		std::string error;
		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();
			
			return;
		}

		m_sql->Disconnect();		
	}	
}

bool active_sip::ActiveSession::IsExistTalkCallOperator(const std::string &_phone)
{
	const std::string query = "select count(phone) from queue where phone = '" + _phone
								+ "' and date_time > '" + GetCurrentStartDay()
								+ "' order by date_time desc limit 1";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);
		
		m_sql->Disconnect();
		
		// ошибка считаем что есть запись	
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

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

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		
		// ошибка считаем что нет записи
		return -1;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);
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

	std::string error;
	// найдем операторов которые по БД числяться в onHold
	if (!GetActiveOnHold(onHoldList, error))
	{
		error += METHOD_NAME;
		m_log.ToFile(ELogType::Error, error);

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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();	
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
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
		std::string error;
		if (!DisableHold(hold, error))
		{
			error += METHOD_NAME;
			m_log.ToFile(ELogType::Error, error);

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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

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
	std::string error;

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
			if (!DisableHold(hold, error))
			{
				error += METHOD_NAME;
				m_log.ToFile(ELogType::Error, error);

				return;
			}

			needNewHoldList = true;
		}
	}
	
	// надо ли пересчитать HoldList
	if (needNewHoldList) 
	{
		if (!GetActiveOnHold(_onHoldList, error))
		{
			error += METHOD_NAME;
			m_log.ToFile(ELogType::Error, error);

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
			std::string error;
			if (!AddHold(sip, error))
			{
				error += METHOD_NAME;
				m_log.ToFile(ELogType::Error, error);
			}
		}
	}
}


