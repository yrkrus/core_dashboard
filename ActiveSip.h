#include <string>
#include <vector>
//#include <iostream>
//#include <sstream>
//#include <list>

#include "IAsteriskData.h"
#include "ISQLConnect.h"
#include "Queue.h"

#ifndef ACTIVESIP_H
#define ACTIVESIP_H

static std::string SESSION_SIP_RESPONSE		= "asterisk -rx \"core show channels concise\"";
static std::string SESSION_QUEUE_RESPONSE = "asterisk -rx \"queue show %queue\"";

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
	
	// структура текущего звонка
	struct ActiveCall 
	{
		std::string phone = "null";			// текущий номер телфеона с которым ведется беседа
		std::string sip = "null";			// внутренний sip который ведет беседу
		std::string talkTime = "null";		// время развговора  //TODO потом в int переделать		
	};
	typedef std::vector<ActiveCall> ActiveCallList;

	// структура onHold
	struct OnHold 
	{
		int id = 0;						// id по БД
		std::string sip = "null";		// sip с которым был разговор
		std::string phone = "null";		// телефон 		
	};
	typedef std::vector<OnHold> OnHoldList;


	class ActiveSession : public IAsteriskData	// класс в котором будет жить данные по активным сессиям операторов 
	{
	public:
		ActiveSession(Queue &_queue);
		~ActiveSession() override;

		void Start() override;
		void Stop() override;
		void Parsing() override;				// разбор сырых данных
		
	private:
		Queue				&m_queueSession;	// ссылка на очереди
		
		OperatorList		m_listOperators;	// TODO может лучше в shared_ptr потом обернуть
		ActiveCallList		m_listCall;			// TODO может лучше в shared_ptr потом обернуть
		SP_SQL				m_sql;		
		IFile				m_queue;			// запрос информации по текущим очередям

		void CreateListActiveSessionOperators();			// активные операторы в линии
		void CreateListActiveSessionCalls();				// активные звонки в линии

		void CreateActiveOperators(const ecQueueNumber _queue);	// найдем активных операторов в линии
		void CreateOperator(const std::string &_lines, Operator &, ecQueueNumber);	// создание структуры Operator					
		std::string FindSipNumber(const std::string &_lines);	// парсинг нахождения активного sip оператора
		bool FindOnHoldStatus(const std::string &_lines);		// парсинг нахождения статуса onHold
		
		void InsertAndUpdateQueueNumberOperators(); // добавление\обновление номена очереди оператора в БД
		bool IsExistListOperators();		// есть ли данные в m_listOperators
		bool IsExistListOperatorsOnHold();	// есть ли данные в m_listOperators.onHold
		bool IsExistListCalls();			// есть ли данные в m_listCall

		bool IsExistOperatorsQueue();	// существует ли хоть 1 запись в БД sip+очередь
		bool IsExistOperatorsQueue(const std::string &_sip, const std::string &_queue);	// существует ли хоть запись в БД sip+очередь
		void ClearOperatorsQueue();		// очистка таблицы operators_queue
		void CheckOperatorsQueue();		// проверка есть ли оператор еще в очереди
		bool GetActiveQueueOperators(OperatorList &_activeList, std::string &_errorDescription); // активные очереди операторов в БД
		void DeleteOperatorsQueue(const std::string &_sip, const std::string &_queue);	// удаление очереди оператора из БД таблицы operators_queue
		void DeleteOperatorsQueue(const std::string &_sip);								// удаление очереди оператора из БД таблицы operators_queue весь sip
		void InsertOperatorsQueue(const std::string &_sip, const std::string &_queue);	// добавление очереди оператору в БД таблицы operators_queue
	
		bool CreateActiveCall(const std::string &_lines, const std::string &_sipNumber, ActiveCall &_caller); // парсинг и нахождение активного звонка с которым разговаривает оператор
		bool CheckActiveCall(const ActiveCall &_caller); // проверка корректности структуры звонка

		void UpdateActiveSessionCalls(); // обновление текущих звонков операторов
		
		void UpdateTalkCallOperator();								// обновление данных таблицы queue о том с кем сейчас разговаривает оператор
		bool IsExistTalkCallOperator(const std::string &_phone);	// существует ли такой номер в таблице queue чтобы добавить sip оператора который с разговор ведет
		int  GetLastTalkCallOperatorID(const std::string &_phone);	// получение последнего ID актуального разговора текущего оператора в таблице queue
	
		// onHold 
		void UpdateOnHoldStatusOperator();					// обновление статуса onHold
		void AddPhoneOnHoldInOperator(Operator &); // добавление номера телефона который на onHold сейчас		
		bool GetActiveOnHold(OnHoldList &_onHoldList, std::string &_errorDescription);	// получение всех onHold стаутсов которые есть в БД
		
		void DisableOnHold(const OnHoldList &_onHoldList);		// очистка всех операторов которые в статусе onHold по БД 
		bool DisableHold(const OnHold &_hold, std::string &_errorDescription);	// отключение onHold в БД
		bool AddHold(const Operator&, std::string &_errorDescription);	// добавление нового onHold в БД

		void CheckOnHold(OnHoldList &_onHoldList);		// Основная проверка отключение\добавление onHold 
	};

}


namespace ACTIVE_SIP_old
{
	/*enum Currentfind
	{
		phone_find,
		internal_sip_find,
		talk_time_find,
	};*/

	//struct Pacients_old
	//{
	//	std::string phone		 { "null" };	// текущий номер телфеона с которым ведется беседа
	//	std::string internal_sip { "null" };	// внутренний sip который ведет беседу
	//	std::string talk_time	 { "null" };    // время развговора  потом в int переделать		
	//};

	struct Operators_old
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

	//class Parsing_old
	//{
	//public:
	//	Parsing_old(const char *fileActiveSip);
	//	~Parsing_old();

	//	void show(bool silent = false);
	//	bool isExistList();
	//	//bool isExistListActiveOperators();
	//	//void createListActiveOperators();

	//	void updateData();										// добавление данных в БД	
	//	std::vector<Operators> getListOperators();				// получение текущего значения со списком активных операторов		

	//private:
	//	std::string findParsing(std::string str, Currentfind find, const std::string &number_operator);		// парсинг
	//	std::string findNumberSip(std::string &str);														// парсинг нахождения активного sip оператора
	//	bool findOnHold(const std::string &str);												    		// парсинг нахождения статуса onHold 


	//	void findActiveOperators(const char *fileOperators, std::string queue);  							// парсинг #2 (для activeoperaots) 

	//	void insert_updateQueueNumberOperators();														    // добавление номена очереди оператора
	//	bool isExistQueueOperators_old();																	    // есть ли операторы в очередях
	//	void clearQueueNumberOperators();																	// очистка номеров очереди операторов

	//	
	//	void clearListOperatorsPhoneOnHold();																// очистка текщего листа phoneOnHold
	//	bool getSipIsOnHold(std::string sip);																// текущее состояние sip (разговор или onHold)

	//	std::vector<Pacients_old> active_sip_list;
	//	std::vector<Operators>list_operators;		
	//};	
	
	class OnHold_old : public Operators_old
	{
	public:
		OnHold_old() = default;
		virtual ~OnHold_old() = default;

		int id{ 0 };
		std::string date_time_start{ "null" };
		std::string date_time_stop{ "null" };
		size_t hash{ 0 };
		std::string phone{ "" };
	};
	
}





#endif // ACTIVESIP_H
