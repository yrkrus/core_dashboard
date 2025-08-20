#ifndef ACTIVESIP_H
#define ACTIVESIP_H

#include <string>
#include <vector>
#include "IAsteriskData.h"
#include "ISQLConnect.h"
#include "Queue.h"


static std::string SESSION_SIP_RESPONSE		= "asterisk -rx \"core show channels concise\"";
static std::string SESSION_QUEUE_RESPONSE	= "asterisk -rx \"queue show %queue\"";

class Queue;
using SP_Queue = std::shared_ptr<Queue>;
enum class ecQueueNumber;
using QueueList = std::vector<ecQueueNumber>;


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
	using OperatorList = std::vector<Operator>;
	
	// структура текущего звонка
	struct ActiveCall 
	{
		std::string phone;			// текущий номер телфеона с которым ведется беседа
		std::string sip;			// внутренний sip который ведет беседу
		std::string talkTime;		// время развговора  //TODO потом в int переделать	
		std::string callID;		// id звонка	
	};
	using ActiveCallList = std::vector<ActiveCall>;

	// структура onHold
	struct OnHold 
	{
		int id = 0;						// id по БД
		std::string sip = "null";		// sip с которым был разговор
		std::string phone = "null";		// телефон 		
	
		inline bool check() const noexcept
		{
			if ((id == 0) ||
				(sip.find("null") != std::string::npos) ||
				(phone.find("null") != std::string::npos))
			{				
				return false;
			}

			return true;
		}
	};
	using OnHoldList = std::vector<OnHold>;


	class ActiveSession : public IAsteriskData	// класс в котором будет жить данные по активным сессиям операторов 
	{
	public:
		ActiveSession(SP_Queue &_queue);
		~ActiveSession() override;

		void Start() override;
		void Stop() override;
		void Parsing() override;				// разбор сырых данных
		
	private:
		SP_Queue	&m_queueSession;	// ссылка на очереди
		
		OperatorList	m_listOperators;	
		ActiveCallList	m_listCall;			
		SP_SQL			m_sql;		
		IFile			m_queue;			// запрос информации по текущим очередям
		Log				m_log;


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
using SP_ActiveSession = std::shared_ptr<active_sip::ActiveSession>;

#endif // ACTIVESIP_H
