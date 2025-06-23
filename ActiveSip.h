#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <list>

#include "IAsteriskData.h"
#include "ISQLConnect.h"
#include "Queue.h"

#ifndef ACTIVESIP_H
#define ACTIVESIP_H

static std::string SESSION_SIP_RESPONSE		= "asterisk -rx \"core show channels concise\"";
static std::string SESSION_QUEUE_OPERATOR	= "asterisk -rx \"queue show %queue\"";

typedef std::vector<ecQueueNumber> QueueList;

namespace active_sip 
{
	// ��������� ��������� 
	struct Operator 
	{
		std::string sipNumber = "null";		// ����� sip ���������
		QueueList	queueList;				// ������� � ������� ����� ���������
		bool isOnHold = false ;             // ��������� �� �������� � ������� OnHold
		std::string phoneOnHold = "null";	// ������� � ������� ���� onHold		
	};
	typedef std::vector<Operator> OperatorList;
	

	class ActiveSession : public IAsteriskData	// ����� � ������� ����� ���� ������ �� �������� ������� ���������� 
	{
	public:
		ActiveSession();
		~ActiveSession() override;

		void Start() override;
		void Stop() override;
		void Parsing() override;				// ������ ����� ������
	private:
		OperatorList		m_listOperators;
		SP_SQL				m_sql;		
		IFile				m_queue;	// ������ ���������� �� ������� �������

		void CreateListActiveSessionOperators();				// �������� ��������� � �����
		void CreateActiveOperators(const ecQueueNumber _queue);	// ������ �������� ���������� � �����
		void CreateOperator(std::string &_lines, Operator &, ecQueueNumber);	// �������� ��������� Operator					
		std::string FindSipNumber(const std::string &_lines);	// ������� ���������� ��������� sip ���������
		bool FindOnHoldStatus(const std::string &_lines);		// ������� ���������� ������� onHold
	};



}


namespace ACTIVE_SIP_old
{
	enum Currentfind
	{
		phone_find,
		internal_sip_find,
		talk_time_find,
	};

	struct Pacients
	{
		std::string phone		 { "null" };	// ������� ����� �������� � ������� ������� ������
		std::string internal_sip { "null" };	// ���������� sip ������� ����� ������
		std::string talk_time	 { "null" };    // ����� ����������  ����� � int ����������		
	};

	struct Operators
	{
		std::string sip_number {"null"};	// ����� sip ���������
		std::vector<std::string> queue;		// ������� � ������� ����� ���������
		bool isOnHold { false };            // ��������� �� �������� � ������� OnHold
		std::string phoneOnHold{ "null" };	// ������� � ������� ���� onHold

	//public:
		void clear() 
		{
			this->sip_number = "null";
			this->queue.clear();
			this->isOnHold = false;
			this->phoneOnHold = "null";
		}
	};

	class Parsing_old
	{
	public:
		Parsing_old(const char *fileActiveSip);
		~Parsing_old();

		void show(bool silent = false);
		bool isExistList();
		bool isExistListActiveOperators();
		void createListActiveOperators();

		void updateData();										// ���������� ������ � ��	
		std::vector<Operators> getListOperators();				// ��������� �������� �������� �� ������� �������� ����������		

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string &number_operator);		// �������
		std::string findNumberSip(std::string &str);														// ������� ���������� ��������� sip ���������
		bool findOnHold(const std::string &str);												    		// ������� ���������� ������� onHold 


		void findActiveOperators(const char *fileOperators, std::string queue);  							// ������� #2 (��� activeoperaots) 

		void insert_updateQueueNumberOperators();														    // ���������� ������ ������� ���������
		bool isExistQueueOperators();																	    // ���� �� ��������� � ��������
		void clearQueueNumberOperators();																	// ������� ������� ������� ����������

		
		void clearListOperatorsPhoneOnHold();																// ������� ������� ����� phoneOnHold
		bool getSipIsOnHold(std::string sip);																// ������� ��������� sip (�������� ��� onHold)

		std::vector<Pacients> active_sip_list;
		std::vector<Operators>list_operators;		
	};	
	

	class OnHold : public Operators
	{
	public:
		OnHold()			= default;
		virtual ~OnHold()	= default;

		int id{ 0 };
		std::string date_time_start{ "null" };
		std::string date_time_stop{ "null" };
		size_t hash{ 0 };
		std::string phone{ "" };
	};
}



#endif // ACTIVESIP_H
