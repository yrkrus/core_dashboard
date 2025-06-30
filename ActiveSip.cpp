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

// ����������
//ACTIVE_SIP_old::Parsing_old::Parsing_old(const char *fileActiveSip)
//{
//	// ������ �������� ���������� � �����
//	//createListActiveOperators();
//	
//	// ���� �� �������� SIP ��������� � ����� 
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
//		// ��������  ��� ���� � 1 ������ ��� ���������� ����� ������� �����������!!! 
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
//									// ���������
//									if (pacient.internal_sip != "null" &&
//										pacient.phone		 != "null" &&
//										pacient.talk_time	 != "null")
//									{
//										active_sip_list.push_back(pacient);
//										break; // ��� ������ ������ while �.�. ����� ������ ������
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

// ����������
//ACTIVE_SIP_old::Parsing_old::~Parsing_old()
//{
//	if (!active_sip_list.empty()) { active_sip_list.clear(); }
//}

// �������� ������ �� ������ � ��������
//bool ACTIVE_SIP_old::Parsing_old::isExistList()
//{
//	return (active_sip_list.empty() ? false : true);
//}

// �������� �������� ���� ���������� ������� � ��� ������ � ����� ���������
//void ACTIVE_SIP_old::Parsing_old::createListActiveOperators()
//{		
//	for (size_t i = 0; i != CONSTANTS::AsteriskQueue::COUNT; ++i) {
//	
//		std::string responce = CONSTANTS::cActiveSipOperatorsResponse;
//
//		// �������� %queue �� ����� �������
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
//	// �������\������� ������� 
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

// ������� ������
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
//		if (str[i] != '!') // ���� ��������� (��������� !)
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
//	  // ������ �� sigmentation fault!
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
//			case ACTIVE_SIP_old::Currentfind::internal_sip_find:	// ���������� ����� SIP (�� ��� � ��� ����� �� �������)
//			{
//				return number_operator;
//				break;
//			}
//			case ACTIVE_SIP_old::Currentfind::talk_time_find:		// ����� ���������
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


// ������� ���������� ��������� sip ���������
//std::string ACTIVE_SIP_old::Parsing_old::findNumberSip(std::string &str)
//{
//	// 6 �.�. lenght("Local/) = 6	
//	return str.substr( str.find_first_of("Local/") + 6,  str.find_first_of("@") - str.find_first_of("Local/") - 6);	
//}


//bool ACTIVE_SIP_old::Parsing_old::findOnHold(const std::string &str)
//{
//	return ((str.find("On Hold") != std::string::npos) ? true : false);	
//}


// // ������� #2 (��� activeoperaots)
//void ACTIVE_SIP_old::Parsing_old::findActiveOperators(const char *fileOperators, std::string queue)
//{
//	
//	std::ifstream file;
//	file.open(fileOperators);
//
//	if (!file.is_open()) {
//		// �� ���������� ������� ���� ������ �� �������
//		return;		
//	}
//
//	std::string line;
//
//	// ��������
//	while (std::getline(file, line))
//	{
//		// �������� �������� CAllers �� ���������
//		if (line.find("Callers") != std::string::npos) {
//			break;
//		}
//		
//		// �������� ���� �� �������� sip
//		if (line.find("Local/") != std::string::npos) {
//			// ������ �������� sip
//			
//			Operators active_operator;
//			active_operator.sip_number = findNumberSip(line);
//			active_operator.queue.push_back(queue);
//			active_operator.isOnHold = findOnHold(line);
//			
//			// �������� ���� �� ��� ����� sip ����� �������� ��� ������ �������
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


// ���������� ������ ������� ���������	
//void ACTIVE_SIP_old::Parsing_old::insert_updateQueueNumberOperators()
//{
//	if (!isExistListActiveOperators())	{			
//		// ������� ������ ������� ����������
//		if (isExistQueueOperators_old()) {
//			clearQueueNumberOperators();
//			return;
//		}			
//	} 	
//
//	SQL_REQUEST::SQL base;	
//
//	// �������� ����� �� ������� �������� ������, ����� ���� ������� sip �� ��
//	if (base.isConnectedBD()) 
//	{
//		base.checkOperatorsQueue(list_operators);
//	}		
//
//
//	// ��������� � ��
//	for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it)
//	{
//		if (base.isConnectedBD())
//		{			
//			// ����� ������� 5000 � 5050
//			for (size_t i = 0; i != it->queue.size(); ++i) {
//				// �������� ���� �� ����� ������
//				if (!base.isExistQueueOperators_old(it->sip_number.c_str(), it->queue[i].c_str())) {
//					
//					//������ ��� ���������
//					base.insertOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str());
//
//				}				
//			}			
//		}
//	}
//}

// ���� �� ��������� � ��������
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



// ������� ������� ������� ����������
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


//���������� ������ � ��
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
//		// ���������� ������ �� ���������� ����������� � ������� onHold
//		/*SQL_REQUEST::SQL base;		TODO ��������
//		if (base.isConnectedBD()) {
//			base.updateOperatorsOnHold(this);
//		}			*/
//
//	}
//	/*else {
//		typedef std::vector<ACTIVE_SIP::OnHold> operators_onhold;
//		
//		// ������ ��� sip ��������� ������� ��������� �� �� � ������� onHold	
//		SQL_REQUEST::SQL base;
//		auto onHold = base.createOnHoldSip();
//
//		if (!onHold->empty()) {
//			// ������� ������ �.�. ���������� ��� ��������
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
//	// ������� ���� phoneOnHold
//	clearListOperatorsPhoneOnHold();	
//	
//	// ������� ������ �� phoneOnHold
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
	
	// ��� ����� �������� ecQueueNumber, ������ ����� ������ ��������� �������
	static const std::vector<ecQueueNumber> queueList =
	{
		ecQueueNumber::e5000,
		ecQueueNumber::e5050,		
	};

	for (const auto &queue : queueList) 
	{
		std::string request = SESSION_QUEUE_RESPONSE;

		// �������� %queue �� ����� �������
		std::string repl = "%queue";
		size_t position = request.find(repl);
		request.replace(position, repl.length(), EnumToString<ecQueueNumber>(queue));

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
void active_sip::ActiveSession::CreateActiveOperators(const ecQueueNumber _queue)
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

void active_sip::ActiveSession::CreateOperator(const std::string &_lines, Operator &_sip, ecQueueNumber _queue)
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
			if (!IsExistOperatorsQueue(sip.sipNumber, EnumToString<ecQueueNumber>(sip.queueList[i]))) 
			{
				//������ ��� ���������
				InsertOperatorsQueue(sip.sipNumber, EnumToString<ecQueueNumber>(sip.queueList[i]));
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
		printf("%s", error.c_str());
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
				DeleteOperatorsQueue(curr_list.sipNumber, EnumToString<ecQueueNumber>(curr_list.queueList[0]));
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

// �������� ������� ��������� �� �� ������� operators_queue
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

// �������� ������� ��������� �� �� ������� operators_queue ���� sip
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

// ���������� ������� ��������� � �� ������� operators_queue
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
							+ getCurrentStartDay() + "' order by date_time desc limit 1";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());

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
		printf("%s", error.c_str());
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
	if (!_onHoldList.empty()) 
	{
		_onHoldList.clear();
	}
	
	// ������ ��� OnHold ������� �� ����������� 
	const std::string query = "select id,sip,phone from operators_ohhold where date_time_stop is NULL order by date_time_start DESC";
	

	if (!m_sql->Request(query, _errorDescription))
	{
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

// ������� ���� ���������� ������� � ������� onHold �� ��
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

// ���������� onHold � ��
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

// ���������� ������ onHold � ��
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

// �������� �������� ����������\���������� onHold 
void active_sip::ActiveSession::CheckOnHold(OnHoldList &_onHoldList)
{
	if (_onHoldList.size() >= 2) 
	{
		std::cout << "test"; // debug ������� �����
	}
	
	// ���� ������� ������������ ���������
	bool needNewHoldList = false; // ���� ���� ��� ����� ����������� HoldList
	std::string error;

	for (const auto &hold : _onHoldList) 
	{
		bool existHold = true; // �� ��������� ������� ��� ���� �������� hold
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
	
	// ���� �� ����������� HoldList
	if (needNewHoldList) 
	{
		if (!GetActiveOnHold(_onHoldList, error))
		{
			printf("%s", error.c_str());
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
				printf("%s", error.c_str());				
			}
		}
	}
}


