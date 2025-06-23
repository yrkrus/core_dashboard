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


// ����������
ACTIVE_SIP_old::Parsing_old::Parsing_old(const char *fileActiveSip)
{
	// ������ �������� ���������� � �����
	createListActiveOperators();
	
	// ���� �� �������� SIP ��������� � ����� 
	if (!isExistListActiveOperators()) 
	{
		std::cout << "\nActive SIP operators is empty!\n";
		return;
	}

	std::ifstream sip;

	sip.open(fileActiveSip);

	if (sip.is_open())
	{
		// ��������  ��� ���� � 1 ������ ��� ���������� ����� ������� �����������!!! 
		if (!list_operators.empty()) {		
			
			std::string line;
			while (std::getline(sip, line))
			{	
				for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {					
				
					if (line.find("Local/" + it->sip_number)	!= std::string::npos) {
						if (line.find("Ring")					== std::string::npos) {
							if (line.find("Down")				== std::string::npos) {
								if (line.find("Outgoing")		== std::string::npos) {

									Pacients pacient;

									pacient.internal_sip = findParsing(line, ACTIVE_SIP_old::Currentfind::internal_sip_find, it->sip_number);
									pacient.phone		 = findParsing(line, ACTIVE_SIP_old::Currentfind::phone_find, it->sip_number);
									pacient.talk_time	 = findParsing(line, ACTIVE_SIP_old::Currentfind::talk_time_find, it->sip_number);


									// ���������
									if (pacient.internal_sip != "null" &&
										pacient.phone		 != "null" &&
										pacient.talk_time	 != "null")
									{
										active_sip_list.push_back(pacient);
										break; // ��� ������ ������ while �.�. ����� ������ ������
									}
								}
							}						
						}			
					}					
				}				
			}
		}		
	}

	sip.close();	
}

// ����������
ACTIVE_SIP_old::Parsing_old::~Parsing_old()
{
	if (!active_sip_list.empty()) { active_sip_list.clear(); }
}

// �������� ������ �� ������ � ��������
bool ACTIVE_SIP_old::Parsing_old::isExistList()
{
	return (active_sip_list.empty() ? false : true);
}

// �������� �������� ���� ���������� ������� � ��� ������ � ����� ���������
void ACTIVE_SIP_old::Parsing_old::createListActiveOperators()
{		
	for (size_t i = 0; i != CONSTANTS::AsteriskQueue::COUNT; ++i) {
	
		std::string responce = CONSTANTS::cActiveSipOperatorsResponse;

		// �������� %queue �� ����� �������
		std::string repl = "%queue";
		size_t position = responce.find(repl);
		responce.replace(position, repl.length(), INTERNALFUNCTION::getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));

		if (!CONSTANTS::DEBUG_MODE) {
			system(responce.c_str());
		}			

		findActiveOperators(CONSTANTS::cActiveSipOperatorsName.c_str(), INTERNALFUNCTION::getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));
	}

	// �������\������� ������� 
	insert_updateQueueNumberOperators();

}


void ACTIVE_SIP_old::Parsing_old::show(bool silent)
{
	std::ostringstream buffer;
	
	if (this->isExistList()) {
		buffer << "Line Active SIP is (" << active_sip_list.size() << ")\n";
		
		if (!silent) {
			buffer << "sip" << "\t    \t" << "phone" << "\t \t" << " talk time" << "\n";

			for (const auto &list : active_sip_list)
			{
				if (!getSipIsOnHold(list.internal_sip)) 
				{
					buffer << list.internal_sip << "\t >> \t" << list.phone << "\t (" << INTERNALFUNCTION::getTalkTime(list.talk_time) << ")\n";
				}
				else 
				{
					buffer << list.internal_sip << " (OnHold) \t" << list.phone << "\t (" << INTERNALFUNCTION::getTalkTime(list.talk_time) << ")\n";
				}				
			}
		}		
	}	
	else 
	{
		buffer << "Active SIP is empty!\n";
	}

	std::cout << buffer.str();
}

// ������� ������
std::string ACTIVE_SIP_old::Parsing_old::findParsing(std::string str, ACTIVE_SIP_old::Currentfind find, const std::string &number_operator)
{
	std::vector<std::string> lines;
	std::string current_str;

	bool isNewLine{ false };

	for (size_t i = 0; i != str.length(); ++i)
	{

		if (isNewLine)
		{
			if (!current_str.empty())
			{
				lines.push_back(current_str);
				current_str.clear();
			}
		}

		if (str[i] != '!') // ���� ��������� (��������� !)
		{
			current_str += str[i];
			isNewLine = false;
		}
		else
		{
			isNewLine = true;
		}
	}

	if (!lines.empty())
	{
	  // ������ �� sigmentation fault!
		if (lines.size() < 10) {
			return "null";
		}
		
		switch (find)
		{
			case ACTIVE_SIP_old::Currentfind::phone_find:
			{
				return INTERNALFUNCTION::phoneParsing(lines[7]);
				break;
			}
			case ACTIVE_SIP_old::Currentfind::internal_sip_find:	// ���������� ����� SIP (�� ��� � ��� ����� �� �������)
			{
				return number_operator;
				break;
			}
			case ACTIVE_SIP_old::Currentfind::talk_time_find:		// ����� ���������
			{
				return lines[9];
				break;
			}			
			default:
			{
				return "null";
				break;
			}
		}
	}
	else {
		return "null";
	}
}


// ������� ���������� ��������� sip ���������
std::string ACTIVE_SIP_old::Parsing_old::findNumberSip(std::string &str)
{
	// 6 �.�. lenght("Local/) = 6	
	return str.substr( str.find_first_of("Local/") + 6,  str.find_first_of("@") - str.find_first_of("Local/") - 6);	
}


bool ACTIVE_SIP_old::Parsing_old::findOnHold(const std::string &str)
{
	return ((str.find("On Hold") != std::string::npos) ? true : false);	
}


// // ������� #2 (��� activeoperaots)
void ACTIVE_SIP_old::Parsing_old::findActiveOperators(const char *fileOperators, std::string queue)
{
	
	std::ifstream file;
	file.open(fileOperators);

	if (!file.is_open()) {
		// �� ���������� ������� ���� ������ �� �������
		return;		
	}

	std::string line;

	// ��������
	while (std::getline(file, line))
	{
		// �������� �������� CAllers �� ���������
		if (line.find("Callers") != std::string::npos) {
			break;
		}
		
		// �������� ���� �� �������� sip
		if (line.find("Local/") != std::string::npos) {
			// ������ �������� sip
			
			Operators active_operator;
			active_operator.sip_number = findNumberSip(line);
			active_operator.queue.push_back(queue);
			active_operator.isOnHold = findOnHold(line);
			
			// �������� ���� �� ��� ����� sip ����� �������� ��� ������ �������
			bool isExistOperator{ false };
			for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {
				
				if (it->sip_number == active_operator.sip_number) {
					isExistOperator = true;
					it->queue.push_back(queue);
					break;
				}
			}
			
			if (!isExistOperator) {
				list_operators.push_back(active_operator);
			}			
		}
	}

	file.close();
}


// ���������� ������ ������� ���������	
void ACTIVE_SIP_old::Parsing_old::insert_updateQueueNumberOperators()
{
	if (!isExistListActiveOperators())	{			
		// ������� ������ ������� ����������
		if (isExistQueueOperators()) {
			clearQueueNumberOperators();
			return;
		}			
	} 	

	SQL_REQUEST::SQL base;	

	// �������� ����� �� ������� �������� ������, ����� ���� ������� sip �� ��
	if (base.isConnectedBD()) 
	{
		base.checkOperatorsQueue(list_operators);
	}		


	// ��������� � ��
	for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it)
	{
		if (base.isConnectedBD())
		{			
			// ����� ������� 5000 � 5050
			for (size_t i = 0; i != it->queue.size(); ++i) {
				// �������� ���� �� ����� ������
				if (!base.isExistOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str())) {
					
					//������ ��� ���������
					base.insertOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str());

				}				
			}			
		}
	}
}

// ���� �� ��������� � ��������
bool ACTIVE_SIP_old::Parsing_old::isExistQueueOperators()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		return base.isExistOperatorsQueue();
	}

	return false;
}



// ������� ������� ������� ����������
void ACTIVE_SIP_old::Parsing_old::clearQueueNumberOperators()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		base.clearOperatorsQueue();
	}
}

void ACTIVE_SIP_old::Parsing_old::clearListOperatorsPhoneOnHold()
{
	for (auto &list : list_operators) {
		list.phoneOnHold = "null";
	}
}

bool ACTIVE_SIP_old::Parsing_old::getSipIsOnHold(std::string sip)
{
	for (auto &list : list_operators) {
		if ( list.sip_number == sip) {			
			return (list.isOnHold ? true : false);			
		}
	}
}


bool ACTIVE_SIP_old::Parsing_old::isExistListActiveOperators()
{
	return (!list_operators.empty() ? true : false );
}


//���������� ������ � ��
void ACTIVE_SIP_old::Parsing_old::updateData()
{
	if (this->isExistList())
	{			
		
		for (std::vector<ACTIVE_SIP_old::Pacients>::iterator it = active_sip_list.begin(); it != active_sip_list.end(); ++it)
		{
			SQL_REQUEST::SQL base;
			if (base.isConnectedBD())
			{
				base.updateQUEUE_SIP(it->phone.c_str(),it->internal_sip.c_str(),it->talk_time.c_str());				
			}
		}
		
		// ���������� ������ �� ���������� ����������� � ������� onHold
		/*SQL_REQUEST::SQL base;		TODO ��������
		if (base.isConnectedBD()) {
			base.updateOperatorsOnHold(this);
		}	*/		

	}
	/*else {
		typedef std::vector<ACTIVE_SIP::OnHold> operators_onhold;
		
		// ������ ��� sip ��������� ������� ��������� �� �� � ������� onHold	
		SQL_REQUEST::SQL base;
		auto onHold = base.createOnHoldSip();

		if (!onHold->empty()) {
			// ������� ������ �.�. ���������� ��� ��������
			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
			{
				SQL_REQUEST::SQL base;
				base.updateOperatorsOnHold(operators_hold->id);
			}
		}

		delete onHold;
	}*/
}

std::vector<ACTIVE_SIP_old::Operators> ACTIVE_SIP_old::Parsing_old::getListOperators()
{
	// ������� ���� phoneOnHold
	clearListOperatorsPhoneOnHold();	
	
	// ������� ������ �� phoneOnHold
	for (auto &operators : list_operators) {
		if (operators.isOnHold) 
		{
				
			for (auto &list_active : active_sip_list)
			{
				if (list_active.internal_sip == operators.sip_number) {
					operators.phoneOnHold = list_active.phone;
					//break;
				}
			}
		}
	}	
	
	return this->list_operators;
}

active_sip::ActiveSession::ActiveSession()
	:IAsteriskData(CONSTANTS::TIMEOUT::ACTIVE_SESSION)
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
	m_listOperators.clear(); // TODO ��� ���� ���������, ����� ��������!!!
	
	// ������ �������� ���������� � �����(������� ������� ��������� �������)
	CreateListActiveSessionOperators();
	
	
	std::string rawLines = GetRawLastData();
	if (rawLines.empty())
	{
		// TODO ��� ��������, ��� ������!
		return;
	}

	

}

// �������� ��������� � �����
void active_sip::ActiveSession::CreateListActiveSessionOperators()
{
	// ��� ����� �������� ecQueueNumber, ������ ����� ������ ��������� �������
	static const std::vector<ecQueueNumber> queueList =
	{
		ecQueueNumber::e5000,
		ecQueueNumber::e5050		
	};

	for (const auto &queue : queueList) 
	{
		std::string request = SESSION_QUEUE_OPERATOR;

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
	
	

	//findActiveOperators(CONSTANTS::cActiveSipOperatorsName.c_str(), INTERNALFUNCTION::getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));


	
	
	

	// �������\������� ������� 
	//insert_updateQueueNumberOperators();
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
		// �������� �������� CAllers �� ���������
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

void active_sip::ActiveSession::CreateOperator(std::string &_lines, Operator &_sip, ecQueueNumber _queue)
{
	_sip.sipNumber = FindSipNumber(_lines);
	_sip.queueList.push_back(_queue);
	_sip.isOnHold = FindOnHoldStatus(_lines);
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

