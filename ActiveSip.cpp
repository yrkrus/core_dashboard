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
	// ��������� ��� �� ��� �� ������� ��������
	m_listCall.clear(); // TODO ��� ���� ���������, ����� �������� ����� �� ������ ��� � �������� � ������� !!
	CreateListActiveSessionCalls();
	
	m_listOperators.clear(); // TODO ��� ���� ���������, ����� ��������, ����� ��� �� ������ � �������� ��� � �������!!!	
	// ������ �������� ���������� � �����(������� ������� ��������� �������)
	CreateListActiveSessionOperators();
	
	
	// ��� �� ���� ����� ������ � �� ���������
	if (IsExistListCalls())
	{
		UpdateActiveSessionCalls();
	}
	else
	{
		if (!IsExistListOperators())
		{
			// ����� ������ �� ��������, ����� ��� ��� �������� ��������
			m_queueSession->UpdateCallSuccess();
		}
	}		
}

// �������� ��������� � �����
void active_sip::ActiveSession::CreateListActiveSessionOperators()
{
	// INFO: ������ ��� ���� ������� ��������� ��������� m_listOperators!
	
	// ��� ����� �������� EQueueNumber, ������ ����� ������ ��������� �������
	static const std::vector<EQueueNumber> queueList =
	{
		EQueueNumber::e5000,
		EQueueNumber::e5050,		
	};

	for (const auto &queue : queueList) 
	{
		std::string request = SESSION_QUEUE_RESPONSE;

		// �������� %queue �� ����� �������
		std::string repl = "%queue";
		size_t position = request.find(repl);
		request.replace(position, repl.length(), EnumToString<EQueueNumber>(queue));

		std::string error;
		m_queue.DeleteRawAll(); // ������� ��� ������� ������ 

		if (!m_queue.CreateData(request, error))
		{
			// TODO ��� �������� ��� ������ (���� ����. �������)
			continue;
		}

		// ������ �������� ���������� � �����
		CreateActiveOperators(queue);	
	}	
	
	// �������\������� ������� 
	InsertAndUpdateQueueNumberOperators();
}

void active_sip::ActiveSession::CreateListActiveSessionCalls()
{
	std::string rawLines = GetRawLastData();
	if (rawLines.empty())
	{
		// TODO ��� ��������, ��� ������!
		return;
	}

	if (!IsExistListOperators()) 
	{
		DeleteRawLastData(); // �� ������ ������
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
				break; // ��� ������ ������ �.�. ����� ������ ������
			}
		}	
	}

	DeleteRawLastData(); // ������� �������������
}

// ������ �������� ���������� � �����
void active_sip::ActiveSession::CreateActiveOperators(const EQueueNumber _queue)
{	
	if (!m_queue.IsExistRaw()) 
	{
		// TODO ��� ������ ���� ������ ������� �� m_listOperators, �������� �� ����!
		return;
	}
	
	std::istringstream ss(m_queue.GetRawLast());
	std::string line;

	while (std::getline(ss, line))
	{
		// �������� �������� Callers �� ���������
		if (line.find("Callers") != std::string::npos)
		{
			break;
		}

		// �������� ���� �� �������� sip
		if (line.find("Local/") != std::string::npos)
		{
			// ������ �������� sip
			Operator sip;
			CreateOperator(line, sip, _queue);			

			// �������� ���� �� ��� ����� sip ����� �������� ��� ������ �������
			bool isExistOperator = false;
			for (auto &listOperators : m_listOperators) 
			{
				if (sip.sipNumber == listOperators.sipNumber) 
				{
					isExistOperator = true;
					listOperators.queueList.push_back(_queue); // �.�. ������� ������ ��� �������
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

	// ���� onHold == true ������� ����� � ������� ������ ����������� ��������
	if (_sip.isOnHold) 
	{
		AddPhoneOnHoldInOperator(_sip);
	}

}

// ������� ���������� ��������� sip ���������
std::string active_sip::ActiveSession::FindSipNumber(const std::string &_lines)
{
	// 6 �.�. lenght("Local/) = 6	
	return _lines.substr(_lines.find_first_of("Local/") + 6, _lines.find_first_of("@") - _lines.find_first_of("Local/") - 6);
}

// ������� ���������� ������� onHold
bool active_sip::ActiveSession::FindOnHoldStatus(const std::string &_lines)
{
	return ((_lines.find("On Hold") != std::string::npos) ? true : false);
}

// ����������\���������� ������ ������� ��������� � ��
void active_sip::ActiveSession::InsertAndUpdateQueueNumberOperators()
{
	// ���� ����� � �������� ������� ���������� �� ����� ��������� ��
	if (!IsExistListOperators())
	{
		// ������� ������ ������� ����������
		if (IsExistOperatorsQueue())
		{
			ClearOperatorsQueue();			
			return;
		}
	}

	// �������� ����� �� ������� �������� ������, ����� ���� ������� sip �� ��
	CheckOperatorsQueue();	


	// ��������� � ��
	for (const auto &sip : m_listOperators) 
	{
		for (size_t i = 0; i != sip.queueList.size(); ++i) 
		{
			if (!IsExistOperatorsQueue(sip.sipNumber, EnumToString<EQueueNumber>(sip.queueList[i]))) 
			{
				//������ ��� ���������
				InsertOperatorsQueue(sip.sipNumber, EnumToString<EQueueNumber>(sip.queueList[i]));
			}
		}
	}	
}

bool active_sip::ActiveSession::IsExistListOperators()
{
	return (!m_listOperators.empty() ? true : false);
}

// ���� �� ������ � m_listOperators.onHold
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

// ���������� �� ���� 1 ������ � �� sip+�������
bool active_sip::ActiveSession::IsExistOperatorsQueue()
{
	std::string error;
	const std::string query = "select count(id) from operators_queue";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);
		
		m_sql->Disconnect();
		// ������ ������� ��� ���� ������
		return true;
	}	

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existOperatorsQueue;
	(std::stoi(row[0]) == 0 ? existOperatorsQueue = false : existOperatorsQueue = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existOperatorsQueue;
}

// ���������� �� ���� ������ � �� sip+�������
bool active_sip::ActiveSession::IsExistOperatorsQueue(const std::string &_sip, const std::string &_queue)
{
	const std::string query = "select count(id) from operators_queue where sip = '" + _sip + "' and queue = '" + _queue + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		// ������ ������� ��� ���� ������
		return true;
	}	

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool exist;
	std::stoi(row[0]) == 0 ? exist = false : exist = true;
	
	mysql_free_result(result);
	m_sql->Disconnect();

	return exist;
}

// ������� ������� operators_queue
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

// �������� ���� �� �������� ��� � �������
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

	// �������� ��������� �� ������ � ������� �� ��
	for (const auto &curr_list : listActiveOperatorsBase)
	{
		bool isExistSip = true;		// ������� ��� sip �� ��������� ����������
		bool isExistQueue = true;	// ������� ��� queue �� ��������� ����������

		for (const auto &memory_list : m_listOperators)
		{
			if (curr_list.sipNumber == memory_list.sipNumber)
			{
				isExistSip = true;

				// �������� ���� �� ����� �������
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
				// ��� sip, ���� ������� �� ��
				isExistSip = false;
			}
		}

		// ��� ������ ����� ������� �� ��
		if (isExistSip)
		{
			if (!isExistQueue)
			{
				// ������� sip + ������� ����������
				DeleteOperatorsQueue(curr_list.sipNumber, EnumToString<EQueueNumber>(curr_list.queueList[0]));
			}
		}
		else
		{
			// ������� ���� sip
			DeleteOperatorsQueue(curr_list.sipNumber);
		}
	}
}

bool active_sip::ActiveSession::GetActiveQueueOperators(OperatorList &_activeList, std::string &_errorDescription)
{
	// ������ ������ �� ��
	_errorDescription = "";
	const std::string query = "select sip,queue from operators_queue";
	
	if (!m_sql->Request(query, _errorDescription))
	{		
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();
		return false;
	}	

	// ���������
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

// �������� ������� ��������� �� �� ������� operators_queue
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

// �������� ������� ��������� �� �� ������� operators_queue ���� sip
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

// ���������� ������� ��������� � �� ������� operators_queue
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
		// ��� ������ ������ �� �����
		return false;
	}

	if (_lines.find("Ring") != std::string::npos ||
		_lines.find("Down") != std::string::npos ||
		_lines.find("Outgoing") != std::string::npos)
	{
		// ��� ������ ������ �� �����
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

		if (_lines[i] != '!') // ���� ��������� (��������� !)
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

// ���������� ������� ������� ����������
void active_sip::ActiveSession::UpdateActiveSessionCalls()
{
	// ���������� ������ ������� queue � ��� � ��� ������ ������������� ��������
	UpdateTalkCallOperator();
	
	// OnHold �������� ����� �������� ������������� � �������� �� ��������� ������
	UpdateOnHoldStatusOperator();	
}

// ���������� ������ ������� queue � ��� � ��� ������ ������������� ��������
void active_sip::ActiveSession::UpdateTalkCallOperator()
{
	if (!IsExistListCalls()) 
	{
		return;
	}

	for (const auto &call : m_listCall)
	{
		// �������� ���� �� ����� �����
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
		
		// ������ ������� ��� ���� ������	
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existQueueSip;
	std::stoi(row[0]) == 0 ? existQueueSip = false : existQueueSip = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return existQueueSip;
}

// ��������� ���������� ID ����������� ��������� �������� ��������� � ������� queue
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
		
		// ������ ������� ��� ��� ������
		return -1;
	}	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);
	int count = std::stoi(row[0]);

	mysql_free_result(result);
	m_sql->Disconnect();

	return count;
}

// ���������� ������ �������� ������� �� onHold ������
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

// ���������� ������� onHold
void active_sip::ActiveSession::UpdateOnHoldStatusOperator()
{
	OnHoldList onHoldList;

	std::string error;
	// ������ ���������� ������� �� �� ��������� � onHold
	if (!GetActiveOnHold(onHoldList, error))
	{
		error += METHOD_NAME;
		m_log.ToFile(ELogType::Error, error);

		return;
	}	

	if (!IsExistListOperators()) 
	{
		// ���� �������� ����������, ������ ����� ��������� �� �� ��� onHold 
		if (!onHoldList.empty()) 
		{
			DisableOnHold(onHoldList);
		}				
		return;
	}	

	// �������� �������� onHold
	CheckOnHold(onHoldList);
}

// ��������� ���� onHold �������� ������� ���� � ��
bool active_sip::ActiveSession::GetActiveOnHold(OnHoldList &_onHoldList, std::string &_errorDescription)
{
	_errorDescription.clear();
	_onHoldList.clear();
	
	// ������ ��� OnHold ������� �� ����������� 
	const std::string query = "select id,sip,phone from operators_ohhold where date_time_stop is NULL order by date_time_start DESC";
	

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();	
		return false;
	}

	// ���������
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

// ������� ���� ���������� ������� � ������� onHold �� ��
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

// ���������� onHold � ��
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

// ���������� ������ onHold � ��
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

// �������� �������� ����������\���������� onHold 
void active_sip::ActiveSession::CheckOnHold(OnHoldList &_onHoldList)
{
	// ���� ������� ������������ ���������
	bool needNewHoldList = false; // ���� ���� ��� ����� ����������� HoldList
	std::string error;

	for (const auto &hold : _onHoldList) 
	{
		bool existHold = false; // �� ��������� ������� ��� ��� ��������� ��������� hold
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
	
	// ���� �� ����������� HoldList
	if (needNewHoldList) 
	{
		if (!GetActiveOnHold(_onHoldList, error))
		{
			error += METHOD_NAME;
			m_log.ToFile(ELogType::Error, error);

			return;
		}
	}	
	
	// �������� ����� ������
	for (const auto &sip : m_listOperators) 
	{
		bool newHold = false; // ����� hold
		if (_onHoldList.empty())  // �.�. ����� � ��, �� ������ ���������
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
				// �������� ���� ����� hold � ��
				if (sip.isOnHold)
				{
					// �� ��������� ������� ��� �� ����� 
					newHold = true;

					if (sip.sipNumber == hold.sip)
					{
						newHold = false;
						break;
					}
				}
			}
		}		

		// ������� ����� hold
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


