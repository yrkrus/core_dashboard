//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.05.2024				//
//    парсинг активных звонков которые в реалтайм	//
//                  ведут разговор					//
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
		std::string phone		 { "null" };	// текущий номер телфеона с которым ведется беседа
		std::string internal_sip { "null" };	// внутренний sip который ведет беседу
		std::string talk_time	 { "null" };    // время развговора  потом в int переделать
	};

	struct Operators
	{
		std::string sip_number {"null"};	// номер sip орератора
		std::vector<std::string> queue;		// очереди в которых сидит орператор

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

		void updateData();		//добавление данных в БД	

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string number_operator);		// парсинг
		std::string findNumberSip(std::string &str);														// парсинг нахождения активного sip оператора

		void findActiveOperators(const char *fileOperators, std::string queue);  							// парсинг #2 (для activeoperaots) 

		void insert_updateQueueNumberOperators();														    // добавление номена очереди оператора
		bool isExistQueueOperators();																	    // есть ли операторы в очередях
		void clearQueueNumberOperators();																	// очистка номеров очереди операторов

		std::vector<Pacients> active_sip_list;
		std::vector<Operators>list_operators;		
	};	
}



#endif // ACTIVESIP_H
