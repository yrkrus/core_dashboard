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

	public:
		void clear() {
			this->sip_number = "null";
			this->queue.clear();
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

		void updateData();		//���������� ������ � ��	

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string number_operator);		// �������
		std::string findNumberSip(std::string &str);														// ������� ���������� ��������� sip ���������

		void findActiveOperators(const char *fileOperators, std::string queue);  							// ������� #2 (��� activeoperaots) 

		void insert_updateQueueNumberOperators();														    // ���������� ������ ������� ���������
		bool isExistQueueOperators();																	    // ���� �� ��������� � ��������
		void clearQueueNumberOperators();																	// ������� ������� ������� ����������

		std::vector<Pacients> active_sip_list;
		std::vector<Operators>list_operators;		
	};	
}



#endif // ACTIVESIP_H
