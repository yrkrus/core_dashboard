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
	// структура оператора 
	struct Operator 
	{
		std::string sipNumber = "null";		// номер sip орератора
		QueueList	queueList;				// очереди в которых сидит орператор
		bool isOnHold = false ;             // находится ли оператор в статусе OnHold
		std::string phoneOnHold = "null";	// телефон с которым идет onHold		
	};
	typedef std::vector<Operator> OperatorList;
	

	class ActiveSession : public IAsteriskData	// класс в котором будет жить данные по активным сессиям операторов 
	{
	public:
		ActiveSession();
		~ActiveSession() override;

		void Start() override;
		void Stop() override;
		void Parsing() override;				// разбор сырых данных
	private:
		OperatorList		m_listOperators;
		SP_SQL				m_sql;		
		IFile				m_queue;	// запрос информауии по текущей очереди

		void CreateListActiveSessionOperators();				// активные операторы в линии
		void CreateActiveOperators(const ecQueueNumber _queue);	// найдем активных операторов в линии
		void CreateOperator(std::string &_lines, Operator &, ecQueueNumber);	// создание структуры Operator					
		std::string FindSipNumber(const std::string &_lines);	// парсинг нахождения активного sip оператора
		bool FindOnHoldStatus(const std::string &_lines);		// парсинг нахождения статуса onHold
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
		std::string phone		 { "null" };	// текущий номер телфеона с которым ведется беседа
		std::string internal_sip { "null" };	// внутренний sip который ведет беседу
		std::string talk_time	 { "null" };    // время развговора  потом в int переделать		
	};

	struct Operators
	{
		std::string sip_number {"null"};	// номер sip орератора
		std::vector<std::string> queue;		// очереди в которых сидит орператор
		bool isOnHold { false };            // находится ли оператор в статусе OnHold
		std::string phoneOnHold{ "null" };	// телефон с которым идет onHold

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

		void updateData();										// добавление данных в БД	
		std::vector<Operators> getListOperators();				// получение текущего значения со списком активных операторов		

	private:
		std::string findParsing(std::string str, Currentfind find, const std::string &number_operator);		// парсинг
		std::string findNumberSip(std::string &str);														// парсинг нахождения активного sip оператора
		bool findOnHold(const std::string &str);												    		// парсинг нахождения статуса onHold 


		void findActiveOperators(const char *fileOperators, std::string queue);  							// парсинг #2 (для activeoperaots) 

		void insert_updateQueueNumberOperators();														    // добавление номена очереди оператора
		bool isExistQueueOperators();																	    // есть ли операторы в очередях
		void clearQueueNumberOperators();																	// очистка номеров очереди операторов

		
		void clearListOperatorsPhoneOnHold();																// очистка текщего листа phoneOnHold
		bool getSipIsOnHold(std::string sip);																// текущее состояние sip (разговор или onHold)

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
