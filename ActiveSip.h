//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.05.2024				//
//    ������� �������� ������� ������� � ��������	//
//                  ����� ��������					//
//													//	
//////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifndef ACTIVESIP_H
#define ACTIVESIP_H


namespace ACTIVE_SIP
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
		void clear() {
			this->sip_number = "null";
			this->queue.clear();
			this->isOnHold = false;
			this->phoneOnHold = "null";
		}
	};

	class Parsing
	{
	public:
		Parsing(const char *fileActiveSip);
		~Parsing();		

		void show(bool silent = false);
		bool isExistList();
		bool isExistListActiveOperators();
		void createListActiveOperators();

		void updateData();										// ���������� ������ � ��	
		std::vector<Operators> getListOperators();				// ��������� �������� �������� �� ������� �������� ����������		

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string number_operator);		// �������
		std::string findNumberSip(std::string &str);														// ������� ���������� ��������� sip ���������
		bool findOnHold(std::string &str);																	// ������� ���������� ������� onHold 


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
