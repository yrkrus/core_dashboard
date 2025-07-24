#include <vector>
#include <string>
#include "Queue.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "utils.h"

using namespace utils;

Queue::Queue()
	:IAsteriskData(CONSTANTS::TIMEOUT::QUEUE)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::QUEUE)
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
	if (FindQueueCallers())	// ���� ����� ������ 
	{		
		// ���� ������ ���������\��������� � ��
		InsertQueueCalls();

		// ��������� ������� (�� ������)
		if (IsExistQueueCalls()) 
		{
			UpdateCalls(m_listQueue);
		}		
	}	

	// ������� �� ����� ������
	DeleteRawLastData();
}

bool Queue::FindQueueCallers()
{	
	m_listQueue.clear(); // ������� ������� ������
	
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

		if (_lines[i] != ' ') // ���� ��������� (��������� ������ ������)
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
		_queueCaller.phone = utils::PhoneParsing(lines[7]);
		_queueCaller.waiting = lines[8];
		_queueCaller.queue = StringToEnum<EQueueNumber>(lines[2]);

		// TODO ��� � ��� ������ ���� �� ������ �� ����� �� ������� 
		if (!CheckCallers(_queueCaller))
		{
			/*LOG_old::LogToFile_old log(LOG_old::eLogType_ERROR);
			std::string err = std::string(__PRETTY_FUNCTION__) +"\t"+ _lines;
			log.add(err);*/

			return false;
		}

		status = true;
	}

	return status;
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
		list.queue != EQueueNumber::e5005	? InsertCall(list)						// ��� ������� �� ����� ���������											
											: InsertCallVirtualOperator(list);		// ��� ���������� ����				
	}	
}

void Queue::UpdateCalls(const QueueCallsList &_callList)
{
	/*if (!IsExistQueueCalls()) 
	{
		return;
	}*/
	
	// ������� � ��������� ������ ���� ������ ��� � �������, �� �� �������� ������ �� ���������
	UpdateCallFail(_callList);

	// ������� � ��������� ������ ����� � ��� ������ �� IVR ����� � �������\�� ������������ ���������
	UpdateCallIvr(_callList);

	// ������� � ��������� ������ ����� �������� ������� ��������� 
	UpdateCallSuccess(_callList);
}

void Queue::InsertCall(const QueueCalls &_call)
{
	std::string error;

	if (IsExistCall(_call.queue, _call.phone)) 
	{
		// ����� ����������, ��������� ������
		unsigned int id = GetLastQueueCallId(_call.phone);
		
		if (!UpdateCall(id, _call, error))
		{
			printf("%s", error.c_str());
		}		
	}
	else 
	{		
		// ��� ������ ������ ���������
		const std::string query = "insert into queue (number_queue,phone,waiting_time) values ('"
									+ EnumToString(_call.queue) + "','"
									+ _call.phone + "','"
									+ _call.waiting + "')";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();						
		}
		
		m_sql->Disconnect();	
	}		
}

// ���������� ������ ������ (����������� �������� ����)
void Queue::InsertCallVirtualOperator(const QueueCalls &_call)
{
	std::string error;

	if (IsExistCallVirtualOperator(_call.queue, _call.phone))
	{
		// ����� ����������, ��������� ������
		unsigned int id = GetLastQueueVirtualOperatorCallId(_call.phone);

		if (!UpdateCallVirualOperator(id, _call, error))
		{
			printf("%s", error.c_str());
		}
	}
	else
	{
		// ��� ������ ������ ���������
		const std::string query = "insert into queue_robot (number_queue,phone,talk_time) values ('"
			+ EnumToString(_call.queue) + "','"
			+ _call.phone + "','"
			+ _call.waiting + "')";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();			
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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return false;
	}
	m_sql->Disconnect();
	
	return true;
}

// ���������� ������������� ������ (����������� ��������)
bool Queue::UpdateCallVirualOperator(int _id, const QueueCalls &_call, std::string &_errorDescription)
{
	std::string error;
	const std::string query = "update queue_robot set talk_time = '"
							+ _call.waiting
							+ "' where phone = '"
							+ _call.phone
							+ "' and id ='" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return false;
	}
	m_sql->Disconnect();

	return true;
}

void Queue::UpdateCallFail(const QueueCallsList &_calls)
{	
	// ������ ������� ������ ������� �� �� ����� ������� ��� ����������
	std::string phoneDoNotTouch;

	for (const auto &list : _calls)
	{
		if (phoneDoNotTouch.empty())
		{
			phoneDoNotTouch = "'" + list.phone + "'";
		}
		else
		{
			phoneDoNotTouch += ",'" + list.phone + "'";	// TODO ��� ��������� 
		}
	}	
	
	// ��������� ������	
	const std::string query = "update queue set fail = '1' where date_time > '"
										+ GetCurrentStartDay()
										+ "' and answered = '0'"
										+ " and sip = '-1' and phone not in (" + phoneDoNotTouch + ")";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return;
	}
	m_sql->Disconnect();
}

//void Queue::UpdateCallFail()
//{
//	// ��������� ������
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

	// ���������� ������ ����� � ��� ������ �� IVR ����� � ������� ��� �� ������������ ���������
void Queue::UpdateCallIvr(const QueueCallsList &_calls)
{	
	if (_calls.empty()) return; // �� ������ ������
	
	// ������ ���� �� ������� � ������ �����������
	UpdateCallIvrToQueue(_calls);

	// ������ ���� �� ������������ ���������
	UpdateCallIvrToVirtualOperator(_calls);	
}

// ������ �� IVR ����� � �������
void Queue::UpdateCallIvrToQueue(const QueueCallsList &_calls)
{
	// ������ ������� ������ ������� ����� ������� ��� ����������
	std::string phoneTouch;

	for (const auto &call : _calls)
	{
		if (call.queue != EQueueNumber::e5005) 
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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return;
	}

	m_sql->Disconnect();
}

// ������ �� IVR ����� �� ������������ ���������
void Queue::UpdateCallIvrToVirtualOperator(const QueueCallsList &_calls)
{
	// ������ ������� ������ ������� ����� ������� ��� ����������
	std::string phoneTouch;

	for (const auto &call : _calls)
	{
		if (call.queue == EQueueNumber::e5005) 
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
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		return;
	}

	m_sql->Disconnect();
}

void Queue::UpdateCallSuccess(const QueueCallsList &_calls)
{
	// �������� ������� ��������� �������� ��������
	UpdateCallSuccessRealOperator(_calls);

	// �������� ������� ��������� ����������� ��������
	UpdateCallSuccessVirtualOperator(_calls);
}

// �������� ������� ��������� �������� ��������
void Queue::UpdateCallSuccessRealOperator(const QueueCallsList &_calls)
{
	CallsInBaseList callsInBase;

	std::string error;
	if (!GetCallsInBase(callsInBase, _calls, error))
	{
		return;
	}

	// ���������	
	for (const auto &call : callsInBase)
	{
		std::string error;
		const std::string query = "update queue set hash = '" + std::to_string(call.hash)
															+ "' where id ='" + call.id
															+ "' and phone ='" + call.phone
															+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();
			continue;
		}
	};

	m_sql->Disconnect();
}

// �������� ������� ��������� ����������� ��������
void Queue::UpdateCallSuccessVirtualOperator(const QueueCallsList &_calls)
{
	CallsInBaseList callsInBase;

	std::string error;
	if (!GetCallsInBaseVirtualOperator(callsInBase, _calls, error))
	{
		return;
	}

	// ���������	
	for (const auto &call : callsInBase)
	{
		std::string error;
		const std::string query = "update queue_robot set hash = '" + std::to_string(call.hash)
									+ "' where id ='" + call.id
									+ "' and phone ='" + call.phone
									+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();
			continue;
		}
	};

	m_sql->Disconnect();
}

// ���������� ������ ����� ��� �������� ���������� �� �����
void Queue::UpdateCallSuccess()
{
	if (IsExistAnyAnsweredCall())
	{
		UpdateAllAnyAnsweredCalls();

		QueueCallsList emptyList;
		UpdateCalls(emptyList);
	}	
}

bool Queue::IsExistCall(EQueueNumber _queue, const std::string &_phone)
{
	std::string error;
	// ���������� ��������� ������� ��������	
	const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString(_queue)
								+ "' and phone = '" + _phone + "'"
								+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"
								+ " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL order by date_time desc limit 1";
	
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		// ��� ������ ������� ��� ������ ����
		return true;
	}	

	// ���������
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
		// ��������� ����� � ������� ������ ��������� ������
		const std::string query = "select count(phone) from queue where number_queue = '" +EnumToString(_queue)
								+ "' and phone = '" + _phone + "'"
								+ " and answered ='0' and fail='0' and sip='-1' and hash is NULL order by date_time desc limit 1";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();			
			// ��� ������ ������� ��� ������ ����
			return true;
		}

		// ���������
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
			// ��� ��������� ��������� ����������
			const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString(_queue)
				+ "' and phone = '" + _phone + "'"
				+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'" //��� ���� ��, �� ����� �� ������������� ��������� 15 ���
				+ " and answered ='0' and fail='1' and sip = '-1' and hash is NULL order by date_time desc limit 1";
		
			if (!m_sql->Request(query, error))
			{
				error += METHOD_NAME + StringFormat("\tquery -> %s", query);
				m_log.ToFile(ELogType::Error, error);

				m_sql->Disconnect();				
				// ��� ������ ������� ��� ������ ����
				return true;
			}

			// ���������
			MYSQL_RES *result = mysql_store_result(m_sql->Get());
			MYSQL_ROW row = mysql_fetch_row(result);
			int countPhone = std::stoi(row[0]);

			mysql_free_result(result);
			m_sql->Disconnect();

			if (countPhone >= 1)
			{
				printf("Boooo!!\n");
				return false; // ������� ��� ����� �����!!!
			}
			else
			{
				// �������� �� �����������, ����� ��� ��� ����������� ����� ���� ��� ���������� ���	
				const std::string query = "select count(phone) from queue where number_queue = '" + EnumToString(_queue)
					+ "' and phone = '" + _phone + "'"
					+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"
					+ " and answered = '1' and fail = '0' and sip <>'-1'"
					+ " and hash is not NULL order by date_time desc limit 1";
				
				if (!m_sql->Request(query, error))
				{
					error += METHOD_NAME + StringFormat("\tquery -> %s", query);
					m_log.ToFile(ELogType::Error, error);

					m_sql->Disconnect();					
					// ��� ������ ������� ��� ������ ����
					return true;
				}

				// ���������
				MYSQL_RES *result = mysql_store_result(m_sql->Get());
				MYSQL_ROW row = mysql_fetch_row(result);

				int countPhone = std::stoi(row[0]);
				mysql_free_result(result);
				m_sql->Disconnect();

				if (countPhone >= 1)
				{
					return false;	// ���� ���� ������, ������ ��������� ������
				}

				return (countPhone == 0 ? false : true);
			}
		}
	}
}

// ���� �� ��� ����� ����� � �� (����������� ��������)
bool Queue::IsExistCallVirtualOperator(EQueueNumber _queue, const std::string &_phone)
{
	std::string error;
	// ���������� ��������� ������� ��������	
	const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
		+ "' and phone = '" + _phone + "'"
		+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"
		+ " and hash is NULL order by date_time desc limit 1";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		// ��� ������ ������� ��� ������ ����
		return true;
	}

	// ���������
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
		// ��� ��������� ��������� ����������
		const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
			+ "' and phone = '" + _phone + "'"
			+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'" //��� ���� ��, �� ����� �� ������������� ��������� 15 ���
			+ " and hash is NULL order by date_time desc limit 1";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();			
			// ��� ������ ������� ��� ������ ����
			return true;
		}

		// ���������
		MYSQL_RES *result = mysql_store_result(m_sql->Get());
		MYSQL_ROW row = mysql_fetch_row(result);
		int countPhone = std::stoi(row[0]);

		mysql_free_result(result);
		m_sql->Disconnect();

		if (countPhone >= 1)
		{			
			return false; // ������� ��� ����� �����!!!
		}
		else
		{
			// �������� �� �����������, ����� ��� ��� ����������� ����� ���� ��� ���������� ���	
			const std::string query = "select count(phone) from queue_robot where number_queue = '" + EnumToString(_queue)
				+ "' and phone = '" + _phone + "'"
				+ " and date_time > '" + GetCurrentDateTimeAfterMinutes(60) + "'"			
				+ " and hash is not NULL order by date_time desc limit 1";

			if (!m_sql->Request(query, error))
			{
				error += METHOD_NAME + StringFormat("\tquery -> %s", query);
				m_log.ToFile(ELogType::Error, error);

				m_sql->Disconnect();				
				// ��� ������ ������� ��� ������ ����
				return true;
			}

			// ���������
			MYSQL_RES *result = mysql_store_result(m_sql->Get());
			MYSQL_ROW row = mysql_fetch_row(result);

			int countPhone = std::stoi(row[0]);
			mysql_free_result(result);
			m_sql->Disconnect();

			if (countPhone >= 1)
			{
				return false;	// ���� ���� ������, ������ ��������� ������
			}

			return (countPhone == 0 ? false : true);
		}		
	}
}

int Queue::GetLastQueueCallId(const std::string &_phone)
{
	const std::string query = "select id from queue where phone = "
							+ _phone + " and date_time > '"
							+ GetCurrentStartDay() + "' order by date_time desc limit 1";

	if (!m_sql->Request(query))
	{
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

// id ������ �� �� � ������(����������� ��������)
int Queue::GetLastQueueVirtualOperatorCallId(const std::string &_phone)
{
	const std::string query = "select id from queue_robot where phone = "
								+ _phone + " and date_time > '"
								+ GetCurrentStartDay() + "' order by date_time desc limit 1";

	if (!m_sql->Request(query))
	{
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

bool Queue::GetCallsInBase(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription)
{
	_errorDescription = "";
	// ������ ������� ������ ������� ����� ������� ��� ����������
	std::string phoneTouch;

	for (const auto &list : _queueCalls)
	{
		if (phoneTouch.empty())
		{
			phoneTouch = "'" + list.phone + "'";
		}
		else
		{
			phoneTouch += ",'" + list.phone + "'"; // TODO ���������
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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query);
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();		
		return false;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query);
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();		
		return false;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
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

// ��������� ������� �� �� (����������� ��������)
bool Queue::GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription)
{
	_errorDescription.clear();
	// ������ ������� ������ ������� ����� ������� ��� ����������
	std::string phoneTouch;

	for (const auto &call : _queueCalls)
	{
		if (call.queue == EQueueNumber::e5005) 
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
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query);
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();		
		return false;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
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

// ��������� ������� �� �� (����������� ��������)
bool Queue::GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, std::string &_errorDescription)
{
	const std::string query = "select id,phone,date_time from queue_robot where date_time > '"
							+ GetCurrentStartDay()
							+ "' and hash is NULL";


	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query);
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
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

//bool Queue::IsExistCallAfter20Hours(std::string &_errorDescription)
//{
//	_errorDescription = "";
//	const std::string query = "select count(phone) from queue where date_time > '"
//								+ getCurrentDateTimeAfter20hours() 
//								+ "' and sip = '-1' and answered = '0' and fail = '0' order by date_time desc ";
//	
//
//	if (!m_sql->Request(query, _errorDescription))
//	{
//		m_sql->Disconnect();
//		printf("%s", _errorDescription.c_str());
//		// ������ ������� ��� ���� ������
//		return true;
//	}	
//
//	// ���������
//	MYSQL_RES *result = mysql_store_result(m_sql->Get());
//	MYSQL_ROW row = mysql_fetch_row(result);
//
//	bool existQueueAfter20hours;
//	(std::stoi(row[0]) == 0 ? existQueueAfter20hours = false : existQueueAfter20hours = true);
//
//	mysql_free_result(result);
//	m_sql->Disconnect();
//
//	return existQueueAfter20hours;	
//}

//void Queue::UpdateCallsAfter20hours()
//{
//	UpdateCallFail();
//}

bool Queue::IsExistAnyAnsweredCall()
{
	std::string error;
	const std::string query = "select count(id) from queue where date_time > '"
							  + GetCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";
	
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();		
		// ������ ������� ��� ���� ������
		return true;
	}	

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

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

	// ���������
	for (const auto &call : callsList)
	{
		const std::string query = "update queue set hash = '" + std::to_string(call.hash)
									+ "' where id ='" + call.id
									+ "' and phone ='" + call.phone
									+ "' and date_time = '" + call.date_time + "'";

		if (!m_sql->Request(query, error))
		{
			error += METHOD_NAME + StringFormat("\tquery -> %s", query);
			m_log.ToFile(ELogType::Error, error);

			m_sql->Disconnect();						
			continue;
		}

	}

	m_sql->Disconnect();
}


