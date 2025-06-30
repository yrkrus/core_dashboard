#include "ActiveSip.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "SQLRequest.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <sstream>

using namespace INTERNALFUNCTION;

// коструктор
//ACTIVE_SIP_old::Parsing_old::Parsing_old(const char *fileActiveSip)
//{
//	// найдем активных операторов в линии
//	//createListActiveOperators();
//	
//	// есть ли активные SIP операторы в линии 
//	/*if (!isExistListActiveOperators()) 
//	{
//		std::cout << "\nActive SIP operators is empty!\n";
//		return;
//	}*/
//
//	std::ifstream sip;
//
//	sip.open(fileActiveSip);
//
//	if (sip.is_open())
//	{
//		// разберем  ТУТ ПОКА в 1 потоке все происходит потом сделать многопочную!!! 
//		if (!list_operators.empty()) {		
//			
//			std::string line;
//			while (std::getline(sip, line))
//			{	
//				for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {					
//				
//					if (line.find("Local/" + it->sip_number)	!= std::string::npos) {
//						if (line.find("Ring")					== std::string::npos) {
//							if (line.find("Down")				== std::string::npos) {
//								if (line.find("Outgoing")		== std::string::npos) {
//
//									Pacients_old pacient;
//
//									pacient.internal_sip = findParsing(line, ACTIVE_SIP_old::Currentfind::internal_sip_find, it->sip_number);
//									pacient.phone		 = findParsing(line, ACTIVE_SIP_old::Currentfind::phone_find, it->sip_number);
//									pacient.talk_time	 = findParsing(line, ACTIVE_SIP_old::Currentfind::talk_time_find, it->sip_number);
//
//
//									// добавляем
//									if (pacient.internal_sip != "null" &&
//										pacient.phone		 != "null" &&
//										pacient.talk_time	 != "null")
//									{
//										active_sip_list.push_back(pacient);
//										break; // нет смысла дальше while т.к. нашли нужные данные
//									}
//								}
//							}						
//						}			
//					}					
//				}				
//			}
//		}		
//	}
//
//	sip.close();	
//}

// деструткор
//ACTIVE_SIP_old::Parsing_old::~Parsing_old()
//{
//	if (!active_sip_list.empty()) { active_sip_list.clear(); }
//}

// проверка пустой ли список в номерами
//bool ACTIVE_SIP_old::Parsing_old::isExistList()
//{
//	return (active_sip_list.empty() ? false : true);
//}

// получаем активный лист операторов которые у нас сейчас в линии находятся
//void ACTIVE_SIP_old::Parsing_old::createListActiveOperators()
//{		
//	for (size_t i = 0; i != CONSTANTS::AsteriskQueue::COUNT; ++i) {
//	
//		std::string responce = CONSTANTS::cActiveSipOperatorsResponse;
//
//		// заменяем %queue на номер очереди
//		std::string repl = "%queue";
//		size_t position = responce.find(repl);
//		responce.replace(position, repl.length(), INTERNALFUNCTION::getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));
//
//		if (!CONSTANTS::DEBUG_MODE) {
//			system(responce.c_str());
//		}			
//
//		findActiveOperators(CONSTANTS::cActiveSipOperatorsName.c_str(), INTERNALFUNCTION::getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));
//	}
//
//	// добавим\обновим очереди 
//	// insert_updateQueueNumberOperators();
//
//}


//void ACTIVE_SIP_old::Parsing_old::show(bool silent)
//{
//	std::ostringstream buffer;
//	
//	if (this->isExistList()) {
//		buffer << "Line Active SIP is (" << active_sip_list.size() << ")\n";
//		
//		if (!silent) {
//			buffer << "sip" << "\t    \t" << "phone" << "\t \t" << " talk time" << "\n";
//
//			for (const auto &list : active_sip_list)
//			{
//				if (!getSipIsOnHold(list.internal_sip)) 
//				{
//					buffer << list.internal_sip << "\t >> \t" << list.phone << "\t (" << INTERNALFUNCTION::getTalkTime(list.talk_time) << ")\n";
//				}
//				else 
//				{
//					buffer << list.internal_sip << " (OnHold) \t" << list.phone << "\t (" << INTERNALFUNCTION::getTalkTime(list.talk_time) << ")\n";
//				}				
//			}
//		}		
//	}	
//	else 
//	{
//		buffer << "Active SIP is empty!\n";
//	}
//
//	std::cout << buffer.str();
//}

// парсинг строки
//std::string ACTIVE_SIP_old::Parsing_old::findParsing(std::string str, ACTIVE_SIP_old::Currentfind find, const std::string &number_operator)
//{
//	std::vector<std::string> lines;
//	std::string current_str;
//
//	bool isNewLine{ false };
//
//	for (size_t i = 0; i != str.length(); ++i)
//	{
//
//		if (isNewLine)
//		{
//			if (!current_str.empty())
//			{
//				lines.push_back(current_str);
//				current_str.clear();
//			}
//		}
//
//		if (str[i] != '!') // ищем разделить (разделить !)
//		{
//			current_str += str[i];
//			isNewLine = false;
//		}
//		else
//		{
//			isNewLine = true;
//		}
//	}
//
//	if (!lines.empty())
//	{
//	  // защита от sigmentation fault!
//		if (lines.size() < 10) {
//			return "null";
//		}
//		
//		switch (find)
//		{
//			case ACTIVE_SIP_old::Currentfind::phone_find:
//			{
//				return INTERNALFUNCTION::phoneParsing(lines[7]);
//				break;
//			}
//			case ACTIVE_SIP_old::Currentfind::internal_sip_find:	// внутренний номер SIP (мы его и так знаем из функции)
//			{
//				return number_operator;
//				break;
//			}
//			case ACTIVE_SIP_old::Currentfind::talk_time_find:		// время разговора
//			{
//				return lines[9];
//				break;
//			}			
//			default:
//			{
//				return "null";
//				break;
//			}
//		}
//	}
//	else {
//		return "null";
//	}
//}


// парсинг нахождения активного sip оператора
//std::string ACTIVE_SIP_old::Parsing_old::findNumberSip(std::string &str)
//{
//	// 6 т.к. lenght("Local/) = 6	
//	return str.substr( str.find_first_of("Local/") + 6,  str.find_first_of("@") - str.find_first_of("Local/") - 6);	
//}


//bool ACTIVE_SIP_old::Parsing_old::findOnHold(const std::string &str)
//{
//	return ((str.find("On Hold") != std::string::npos) ? true : false);	
//}


// // парсинг #2 (для activeoperaots)
//void ACTIVE_SIP_old::Parsing_old::findActiveOperators(const char *fileOperators, std::string queue)
//{
//	
//	std::ifstream file;
//	file.open(fileOperators);
//
//	if (!file.is_open()) {
//		// не получается открыть файл уходим из функции
//		return;		
//	}
//
//	std::string line;
//
//	// разберем
//	while (std::getline(file, line))
//	{
//		// заносить активные CAllers не требуется
//		if (line.find("Callers") != std::string::npos) {
//			break;
//		}
//		
//		// проверим есть ли активный sip
//		if (line.find("Local/") != std::string::npos) {
//			// найдем активный sip
//			
//			Operators active_operator;
//			active_operator.sip_number = findNumberSip(line);
//			active_operator.queue.push_back(queue);
//			active_operator.isOnHold = findOnHold(line);
//			
//			// проверим есть ли уже такой sip чтобы добавить ему только очередь
//			bool isExistOperator{ false };
//			for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {
//				
//				if (it->sip_number == active_operator.sip_number) {
//					isExistOperator = true;
//					it->queue.push_back(queue);
//					break;
//				}
//			}
//			
//			if (!isExistOperator) {
//				list_operators.push_back(active_operator);
//			}			
//		}
//	}
//
//	file.close();
//}


// добавление номена очереди оператора	
//void ACTIVE_SIP_old::Parsing_old::insert_updateQueueNumberOperators()
//{
//	if (!isExistListActiveOperators())	{			
//		// очищаем номера очереди операторов
//		if (isExistQueueOperators_old()) {
//			clearQueueNumberOperators();
//			return;
//		}			
//	} 	
//
//	SQL_REQUEST::SQL base;	
//
//	// проверим вдруг из очереди оператор убежал, тогда надо удалить sip из БД
//	if (base.isConnectedBD()) 
//	{
//		base.checkOperatorsQueue(list_operators);
//	}		
//
//
//	// добавляем в БД
//	for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it)
//	{
//		if (base.isConnectedBD())
//		{			
//			// вдруг очередь 5000 и 5050
//			for (size_t i = 0; i != it->queue.size(); ++i) {
//				// проверим есть ли такая запись
//				if (!base.isExistQueueOperators_old(it->sip_number.c_str(), it->queue[i].c_str())) {
//					
//					//записи нет добавляем
//					base.insertOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str());
//
//				}				
//			}			
//		}
//	}
//}

// есть ли операторы в очередях
//bool ACTIVE_SIP_old::Parsing_old::isExistQueueOperators_old()
//{
//	SQL_REQUEST::SQL base;
//	if (base.isConnectedBD())
//	{
//		return base.isExistOperatorsQueue_old();
//	}
//
//	return false;
//}



// очистка номеров очереди операторов
//void ACTIVE_SIP_old::Parsing_old::clearQueueNumberOperators()
//{
//	SQL_REQUEST::SQL base;
//	if (base.isConnectedBD())
//	{
//		base.clearOperatorsQueue();
//	}
//}

//void ACTIVE_SIP_old::Parsing_old::clearListOperatorsPhoneOnHold()
//{
//	for (auto &list : list_operators) {
//		list.phoneOnHold = "null";
//	}
//}

//bool ACTIVE_SIP_old::Parsing_old::getSipIsOnHold(std::string sip)
//{
//	for (auto &list : list_operators) {
//		if ( list.sip_number == sip) {			
//			return (list.isOnHold ? true : false);			
//		}
//	}
//}


//bool ACTIVE_SIP_old::Parsing_old::isExistListActiveOperators()
//{
//	return (!list_operators.empty() ? true : false );
//}


//добавление данных в БД
//void ACTIVE_SIP_old::Parsing_old::updateData()
//{
//	if (this->isExistList())
//	{			
//		
//		for (std::vector<ACTIVE_SIP_old::Pacients_old>::iterator it = active_sip_list.begin(); it != active_sip_list.end(); ++it)
//		{
//			SQL_REQUEST::SQL base;
//			if (base.isConnectedBD())
//			{
//				base.updateQUEUE_SIP(it->phone.c_str(),it->internal_sip.c_str(),it->talk_time.c_str());				
//			}
//		}
//		
//		// обновление данных по операторам находящимся в статусе onHold
//		/*SQL_REQUEST::SQL base;		TODO починить
//		if (base.isConnectedBD()) {
//			base.updateOperatorsOnHold(this);
//		}			*/
//
//	}
//	/*else {
//		typedef std::vector<ACTIVE_SIP::OnHold> operators_onhold;
//		
//		// найдем все sip операторы которые числяться по БД в статусе onHold	
//		SQL_REQUEST::SQL base;
//		auto onHold = base.createOnHoldSip();
//
//		if (!onHold->empty()) {
//			// очищаем список т.к. операторов нет аквтиных
//			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
//			{
//				SQL_REQUEST::SQL base;
//				base.updateOperatorsOnHold(operators_hold->id);
//			}
//		}
//
//		delete onHold;
//	}*/
//}

//std::vector<ACTIVE_SIP_old::Operators> ACTIVE_SIP_old::Parsing_old::getListOperators()
//{
//	// очищаем лист phoneOnHold
//	clearListOperatorsPhoneOnHold();	
//	
//	// обновим данные по phoneOnHold
//	for (auto &operators : list_operators) {
//		if (operators.isOnHold) 
//		{
//				
//			for (auto &list_active : active_sip_list)
//			{
//				if (list_active.internal_sip == operators.sip_number) {
//					operators.phoneOnHold = list_active.phone;
//					//break;
//				}
//			}
//		}
//	}	
//	
//	return this->list_operators;
//}

active_sip::ActiveSession::ActiveSession(const SP_Queue &_queue)
	:IAsteriskData(CONSTANTS::TIMEOUT::ACTIVE_SESSION)
	, m_queueSession(_queue)
	, m_sql(std::make_shared<ISQLConnect>(false))
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
	
	// без цикла очередей ecQueueNumber, просто берем нужные статичные очереди
	static const std::vector<ecQueueNumber> queueList =
	{
		ecQueueNumber::e5000,
		ecQueueNumber::e5050,		
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
			if (i == 0)
			{
				activeOperator.sipNumber = row[i];
			}
			else if (i == 1)
			{
				activeOperator.queueList.push_back(StringToEnum<ecQueueNumber>(row[i]));
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
		m_sql->Disconnect();
		printf("%s", error.c_str());
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
		m_sql->Disconnect();
		printf("%s", error.c_str());
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
		m_sql->Disconnect();
		printf("%s", error.c_str());
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
			m_sql->Disconnect();
			printf("%s", error.c_str());
			return;
		}

		m_sql->Disconnect();		
	}	
}

bool active_sip::ActiveSession::IsExistTalkCallOperator(const std::string &_phone)
{
	const std::string query = "select count(phone) from queue where phone = '" + _phone
								+ "' and date_time > '" + getCurrentStartDay()
								+ "' order by date_time desc limit 1";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());

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
							+ getCurrentStartDay() + "' order by date_time desc limit 1";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());

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
		printf("%s", error.c_str());
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
	if (!_onHoldList.empty()) 
	{
		_onHoldList.clear();
	}
	
	// найдем все OnHold которые не завершенные 
	const std::string query = "select id,sip,phone from operators_ohhold where date_time_stop is NULL order by date_time_start DESC";
	

	if (!m_sql->Request(query, _errorDescription))
	{
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
			if (i == 0)			// id
			{
				onHold.id = std::atoi(row[i]);
			}
			else if (i == 1)	// sip
			{
				onHold.sip = row[i];
			}			
			else if (i == 2)	// phone
			{
				onHold.phone = row[i];
			}
		}

		_onHoldList.push_back(onHold);
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
			printf("%s", error.c_str());
			continue;
		}
	}
}

// отключение onHold в БД
bool active_sip::ActiveSession::DisableHold(const OnHold &_hold, std::string &_errorDescription)
{
	_errorDescription.clear();
	const std::string query = "update operators_ohhold set date_time_stop = '" + getCurrentDateTime() 
							+ "' where id = '" + std::to_string(_hold.id)
							+ "' and sip = '" + _hold.sip + "'";

	if (!m_sql->Request(query, _errorDescription))
	{
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
		m_sql->Disconnect();
		return false;
	}
	m_sql->Disconnect();

	return true;
}

// Основная проверка отключение\добавление onHold 
void active_sip::ActiveSession::CheckOnHold(OnHoldList &_onHoldList)
{
	if (_onHoldList.size() >= 2) 
	{
		std::cout << "test"; // debug удалить потом
	}
	
	// надо сначало существующие проверить
	bool needNewHoldList = false; // флаг того что нужно пересчитать HoldList
	std::string error;

	for (const auto &hold : _onHoldList) 
	{
		bool existHold = true; // по умолчанию считаем что есть активный hold
		for (const auto &sip : m_listOperators)
		{
			if (hold.sip == sip.sipNumber) 
			{
				if (!sip.isOnHold) 
				{
					existHold = false;
					break;
				}				
			}
		}

		if (!existHold) 
		{			
			if (!DisableHold(hold, error))
			{
				printf("%s", error.c_str());
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
			printf("%s", error.c_str());
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
				printf("%s", error.c_str());				
			}
		}
	}
}


