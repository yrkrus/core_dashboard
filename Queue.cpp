#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <sstream>

#include "Queue.h"
#include "InternalFunction.h"
#include "SQLRequest.h"

using namespace INTERNALFUNCTION;

// коструктор
//QUEUE_OLD::Parsing::Parsing(const char *fileQueue)
//{
//	std::ifstream filequeue;
//
//	filequeue.open(fileQueue);
//
//	if (filequeue.is_open())
//	{
//		//std::cout << "open file... OK\n";
//
//		std::string line;
//
//		// разберем
//		while (std::getline(filequeue, line))
//		{
//			Pacients_old pacient;
//
//			pacient.phone	= findParsing(line, QUEUE_OLD::Currentfind::phone_find);
//			pacient.waiting = findParsing(line, QUEUE_OLD::Currentfind::waiting_find);
//			pacient.queue	= findParsing(line, QUEUE_OLD::Currentfind::queue_find);
//
//			// добавляем
//			if (pacient.phone	!= "null" && 
//				pacient.waiting != "null" && 
//				pacient.queue	!= "null")
//			{
//				pacient_list.push_back(pacient);
//			}
//		}
//	}
//
//	filequeue.close();
//	
//}

// деструткор
//QUEUE::Parsing::~Parsing()
//{
//	if (!pacient_list.empty())
//	{
//		pacient_list.clear();
//	}
//}

// проверка пустой ли список в номерами
//bool QUEUE_OLD::Parsing::isExistList()
//{
//	return (pacient_list.empty() ? false : true);
//}

//void QUEUE_OLD::Parsing::show(bool silent)
//{
//	std::ostringstream buffer;
//	
//	if (isExistList())
//	{
//		buffer << "Line QUEUE is (" << pacient_list.size() << ")\n";
//		
//		if (!silent) {
//			buffer << "queue" << "\t    \t" << "phone" << "\t \t" << " wait time" << "\n";
//
//			for (std::vector<QUEUE_OLD::Pacients_old>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
//			{
//				buffer << it->queue << "\t >> \t" << it->phone << "\t (" << it->waiting << ")\n";
//			}
//		}	
//			
//	}
//	else
//	{
//		buffer << "QUEUE is empty!\n";
//	}
//
//	 std::cout << buffer.str();
//}


//добавление данных в БД
//void QUEUE_OLD::Parsing::insertData()
//{
//	if (this->isExistList())
//	{
//		SQL_REQUEST::SQL base;
//
//		for (std::vector<QUEUE_OLD::Pacients_old>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
//		{
//			if (base.isConnectedBD())
//			{
//				base.insertQUEUE(it->queue.c_str(), it->phone.c_str(), it->waiting.c_str());
//			}
//		}
//		
//		
//		// находим и обновляем данные если звонок был в очереди, но не дождался ответа от оператора
//		if (base.isConnectedBD()) 
//		{ 
//			base.updateQUEUE_fail(pacient_list);			
//		}	
//
//		// находим и обновляем данные когда у нас звонок из IVR попал в очередь
//		if (base.isConnectedBD())
//		{
//			base.updateIVR_to_queue(pacient_list);
//		}
//
//		// находим и обновляем данные когда разговор успешно состоялся 
//		if (base.isConnectedBD()) 
//		{
//			base.updateQUEUE_hash(pacient_list);
//		}
//
//	}	
//}

// проверка есть ли не отвеченные записи после 20:00
//bool QUEUE_OLD::Parsing::isExistQueueAfter20hours()
//{
//	SQL_REQUEST::SQL base;
//
//	if (base.isConnectedBD()) {
//		return base.isExistQueueAfter20hours();
//	}
//
//	return false;
//}

// обновление данных если звонок пришел того как нет активных операторов на линии
//void QUEUE_OLD::Parsing::updateQueueAfter20hours()
//{
//	SQL_REQUEST::SQL base;
//
//	if (base.isConnectedBD())
//	{
//		base.updateQUEUE_fail();
//	}
//}


// проверка если ли номера по которым закончили разгвоаривать, но не успели обработать
//bool QUEUE_OLD::Parsing::isExistAnsweredAfter20hours()
//{
//	SQL_REQUEST::SQL base;
//
//	if (base.isConnectedBD())
//	{
//		return base.isExistAnsweredAfter20hours();
//	}
//
//	return false;
//}

// обновление данных когда закончили разговаривать и ушли с линии
//void QUEUE_OLD::Parsing::updateAnsweredAfter20hours()
//{
//	SQL_REQUEST::SQL base;
//
//	if (base.isConnectedBD())
//	{
//		base.updateAnswered_fail();
//	}
//}



// парсинг строки
//std::string QUEUE_OLD::Parsing::findParsing(std::string str, Currentfind find)
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
//		if (str[i] != ' ') // ищем разделить (разделить пустая строка)
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
//		switch (find)
//		{
//		case QUEUE_OLD::Currentfind::phone_find:
//		{
//			return phoneParsing(lines[7]);	
//			break;
//		}
//		case QUEUE_OLD::Currentfind::waiting_find:
//		{
//			return lines[8];
//			break;
//		}
//		case QUEUE_OLD::Currentfind::queue_find: {
//			return lines[2];
//			break;
//		}
//		default:
//		{
//			return "null";
//			break;
//		}
//		}
//	}
//	else {
//		return "null";
//	}
//}

Queue::Queue()
	:IAsteriskData(CONSTANTS::TIMEOUT::QUEUE)
	, m_sql(std::make_shared<ISQLConnect>(false))
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
}

void Queue::Parsing()
{	
	if (FindQueueCallers())	// ждем сырые данные 
	{		
		// есть данные добавляем\обновляем в БД
		InsertQueueCalls();

		// обновляем очереди (по списку)
		UpdateCalls();
	}	

	
	// TODO сделать!!!!
		//std::string error;

		//// проверим время вдруг кто то позвонил после 20:00:00 и был в очереди, тогда надо пройтись 1 раз по БД
		//if (IsExistCallAfter20Hours(error))
		//{
		//	// есть потеряшки, обновляем данные по ним
		//	UpdateCallsAfter20hours();			
		//}
		//// проверим есть ли не про hash'нные номера, когда оператор уже закончил разговор и ушел из линии
		//if (IsExistCallAnsweredAfter20hours(error))
		//{		
		//	UpdateCallAnsweredAfter20hours();
		//}
	

	// удаляем из сырых данных
	DeleteRawLastData();
}

bool Queue::FindQueueCallers()
{	
	m_listQueue.clear(); // обнулим текущий список
	
	std::string rawLines = GetRawLastData();
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

		if (_lines[i] != ' ') // ищем разделить (разделить пустая строка)
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
		_queueCaller.phone = INTERNALFUNCTION::PhoneParsing(lines[7]);
		_queueCaller.waiting = lines[8];
		_queueCaller.queue = StringToEnum<ecQueueNumber>(lines[2]);

		// TODO тут в лог запись если не прошел по какой то причине 
		if (!CheckCallers(_queueCaller))
		{
			LOG::LogToFile log(LOG::eLogType_ERROR);
			std::string err = std::string(__PRETTY_FUNCTION__) +"\t"+ _lines;
			log.add(err);

			return false;
		}

		status = true;
	}

	return status;
}

bool Queue::CheckCallers(const QueueCalls &_caller)
{
	// если в phone или waiting есть подстрока "null" 
	// или callerID == eUnknown — сразу false
	if (_caller.phone.find("null") != std::string::npos ||
		_caller.waiting.find("null") != std::string::npos ||
		_caller.queue == ecQueueNumber::eUnknown)
	{
		return false;
	}

	return true;
}

bool Queue::IsExistQueueCalls()
{
	return !m_listQueue.empty() ? true : false;
}

void Queue::InsertQueueCalls()
{	
	for (const auto &list : m_listQueue) 
	{	
		InsertCall(list);		
	}	
}

void Queue::UpdateCalls()
{
	if (!IsExistQueueCalls()) 
	{
		return;
	}
	
	// находим и обновляем данные если звонок был в очереди, но не дождался ответа от оператора
	UpdateCallFail(m_listQueue);

	// находим и обновляем данные когда у нас звонок из IVR попал в очередь
	UpdateCallToIVR(m_listQueue);

	// находим и обновляем данные когда разговор успешно состоялся 
	UpdateCallSuccess(m_listQueue);
}

void Queue::InsertCall(const QueueCalls &_call)
{
	std::string error;

	if (IsExistCall(_call.queue, _call.phone)) 
	{
		// номер существует, обновляем данные
		unsigned int id = GetLastQueueCallId(_call.phone);
		
		if (!UpdateCall(id, _call, error))
		{
			printf("%s", error.c_str());
		}		
	}
	else 
	{		
		// нет такого звонка добавляем
		const std::string query = "insert into queue (number_queue,phone,waiting_time) values ('"
									+ EnumToString(_call.queue) + "','"
									+ _call.phone + "','"
									+ _call.waiting + "')";

		if (!m_sql->Request(query, error))
		{
			m_sql->Disconnect();
			printf("%s", error.c_str());			
		}
		
		m_sql->Disconnect();	
	}		
}

bool Queue::UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription)
{
	std::string error;
	const std::string query = "update queue set waiting_time = '" 
								+ _call.waiting 
								+ "' where phone = '" 
								+ _call.phone 
								+ "' and id ='" + std::to_string(_id) + "'";;
	
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());
		return false;
	}
	m_sql->Disconnect();
	
	return true;
}

void Queue::UpdateCallFail(const QueueCallsList &_calls)
{	
	// найдем текущие номера которые мы не будет трогать при обновлении
	std::string phoneDoNotTouch;

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
										+ getCurrentStartDay()
										+ "' and answered = '0'"
										+ " and sip = '-1' and phone not in (" + phoneDoNotTouch + ")";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());
		return;
	}
	m_sql->Disconnect();
}

void Queue::UpdateCallFail()
{
	// обновляем данные
	const std::string query = "update queue set fail = '1' where date_time > '" 
								+ getCurrentDateTimeAfter20hours() 
								+ "' and answered = '0' and sip = '-1' ";

	if (!m_sql->Request(query))
	{
		m_sql->Disconnect();		
		return;
	}	

	m_sql->Disconnect();
}

void Queue::UpdateCallToIVR(const QueueCallsList &_calls)
{
	// найдем текущие номера которые будем трогать при обновлении
	std::string phoneTouch;

	for (const auto &list : _calls)
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
	
	const std::string query = "update ivr set to_queue = '1' where date_time > '"
								+ getCurrentDateTimeAfterMinutes(5)
								+ "' and phone in(" + phoneTouch + ") and to_queue = '0'";
	
	
	std::string error;
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());
		return;
	}
	
	m_sql->Disconnect();	
}

void Queue::UpdateCallSuccess(const QueueCallsList &_calls)
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
			printf("%s", error.c_str());
			m_sql->Disconnect();
			continue;
		}
	};

	m_sql->Disconnect();	
}

bool Queue::IsExistCall(ecQueueNumber _queue, const std::string &_phone)
{
	std::string error;
	// правильней проверять сначало разговор	
	const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString(_queue)
								+ "' and phone = '" + _phone + "'"
								+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'"
								+ " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL order by date_time desc limit 1";
	
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());
		// при ошибке считаем что запись есть
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

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
		const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString(_queue)
								+ "' and phone = '" + _phone + "'"
								+ " and answered ='0' and fail='0' and sip='-1' and hash is NULL order by date_time desc limit 1";

		if (!m_sql->Request(query, error))
		{
			m_sql->Disconnect();
			printf("%s", error.c_str());
			// при ошибке считаем что запись есть
			return true;
		}

		// результат
		MYSQL_RES *result = mysql_store_result(m_sql->Get());
		MYSQL_ROW row = mysql_fetch_row(result);
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
			const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString(_queue)
				+ "' and phone = '" + _phone + "'"
				+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'" //тут типа ок, но время не затрагивается последние 15 мин
				+ " and answered ='0' and fail='1' and sip = '-1' and hash is NULL order by date_time desc limit 1";
		
			if (!m_sql->Request(query, error))
			{
				m_sql->Disconnect();
				printf("%s", error.c_str());
				// при ошибке считаем что запись есть
				return true;
			}

			// результат
			MYSQL_RES *result = mysql_store_result(m_sql->Get());
			MYSQL_ROW row = mysql_fetch_row(result);
			int countPhone = std::stoi(row[0]);

			mysql_free_result(result);
			m_sql->Disconnect();

			if (countPhone >= 1)
			{
				printf("Boooo!!\n");
				return false; // считаем как новый вызов!!!
			}
			else
			{
				// проверка на повторность, вдруг еще раз перезвонили после того как поговорили уже	
				const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString(_queue)
					+ "' and phone = '" + _phone + "'"
					+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'"
					+ " and answered = '1' and fail = '0' and sip <>'-1'"
					+ " and hash is not NULL order by date_time desc limit 1";
				
				if (!m_sql->Request(query, error))
				{
					m_sql->Disconnect();
					printf("%s", error.c_str());
					// при ошибке считаем что запись есть
					return true;
				}

				// результат
				MYSQL_RES *result = mysql_store_result(m_sql->Get());
				MYSQL_ROW row = mysql_fetch_row(result);

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

int Queue::GetLastQueueCallId(const std::string &_phone)
{
	const std::string query = "select id from queue where phone = "
							+ _phone + " and date_time > '"
							+ getCurrentStartDay() + "' order by date_time desc limit 1";

	if (!m_sql->Request(query))
	{
		m_sql->Disconnect();		
		return -1;
	}
	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);
	int count = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return count;
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
									+ getCurrentStartDay()
									+ "' and answered = '1' and fail = '0' and hash is NULL and phone not in(" + phoneTouch + ")";
	} 
	else 
	{
		query = "select id,phone,date_time from queue where date_time > '"
									+ getCurrentStartDay()
									+ "' and answered = '1' and fail = '0' and hash is NULL";
	}	
		
	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		printf("%s", _errorDescription.c_str());
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{

			if (i == 0)
			{
				call.id = row[i];
			}
			else if (i == 1)
			{
				call.phone = row[i];
			}
			else if (i == 2)
			{
				call.date_time = row[i];
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
								+ getCurrentStartDay() 
								+ "' and answered = '1' and fail = '0' and hash is NULL";

	
	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();		
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	bool status = false;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		CallsInBase call;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{

			if (i == 0)
			{
				call.id = row[i];
			}
			else if (i == 1)
			{
				call.phone = row[i];
			}
			else if (i == 2)
			{
				call.date_time = row[i];
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

bool Queue::IsExistCallAfter20Hours(std::string &_errorDescription)
{
	_errorDescription = "";
	const std::string query = "select count(phone) from queue where date_time > '"
								+ getCurrentDateTimeAfter20hours() 
								+ "' and sip = '-1' and answered = '0' and fail = '0' order by date_time desc ";
	

	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		printf("%s", _errorDescription.c_str());
		// ошибка считаем что есть запись
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existQueueAfter20hours;
	(std::stoi(row[0]) == 0 ? existQueueAfter20hours = false : existQueueAfter20hours = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existQueueAfter20hours;	
}

void Queue::UpdateCallsAfter20hours()
{
	UpdateCallFail();
}

bool Queue::IsExistCallAnsweredAfter20hours(std::string &_errorDescription)
{
	const std::string query = "select count(id) from queue where date_time > '"
								+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";
	
	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		printf("%s", _errorDescription.c_str());
		// ошибка считаем что есть запись
		return true;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existAnsweredAfter20;
	(std::stoi(row[0]) == 0 ? existAnsweredAfter20 = false : existAnsweredAfter20 = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existAnsweredAfter20;
}

void Queue::UpdateCallAnsweredAfter20hours()
{	
	std::string error;

	CallsInBaseList callsList;
	if (!GetCallsInBase(callsList, error)) 
	{
		printf("%s", error.c_str());
		return;
	}	

	// обновляем
	for (const auto &list : callsList)
	{
		const std::string query = "update queue set hash = '" + std::to_string(list.hash)
									+ "' where id ='" + list.id
									+ "' and phone ='" + list.phone
									+ "' and date_time = '" + list.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			m_sql->Disconnect();
			printf("%s", error.c_str());			
			continue;
		}

	}

	m_sql->Disconnect();
}


