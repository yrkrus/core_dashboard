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
		bool isOnHold { false };            // находится ли оператор в статусе OnHold

	//public:
		void clear() {
			this->sip_number = "null";
			this->queue.clear();
			this->isOnHold = false;
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

		void updateData();										// добавление данных в БД	
		std::vector<Operators> getListOperators() const;		// получение текущего значения со списком активных операторов

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string number_operator);		// парсинг
		std::string findNumberSip(std::string &str);														// парсинг нахождения активного sip оператора
		bool findOnHold(std::string &str);																	// парсинг нахождения статуса onHold 


		void findActiveOperators(const char *fileOperators, std::string queue);  							// парсинг #2 (для activeoperaots) 

		void insert_updateQueueNumberOperators();														    // добавление номена очереди оператора
		bool isExistQueueOperators();																	    // есть ли операторы в очередях
		void clearQueueNumberOperators();																	// очистка номеров очереди операторов

		bool getSipIsOnHold(std::string sip);																// текущее состояние sip (разговор или onHold)

		std::vector<Pacients> active_sip_list;
		std::vector<Operators>list_operators;		
	};	
	

	class OnHold : public Operators
	{
	public:
		OnHold()	= default;
		virtual ~OnHold()	= default;

		int id{ 0 };
		std::string date_time_start{ "null" };
		std::string date_time_stop{ "null" };
		size_t hash{ 0 };
	};
}



#endif // ACTIVESIP_H
