// Активные звонки ВХОДЯЩИЕ!

#ifndef ACTIVESIP_H
#define ACTIVESIP_H

#include <string>
#include <vector>
#include "../interfaces/IAsteriskData.h"
#include "../interfaces/ISQLConnect.h"
#include "Queue.h"


static std::string SIP_COMMANDS_EXT1		= "ivr-5";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT2		= "Ring";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT3		= "Down";					// пропуск этой записи
static std::string SIP_COMMANDS_EXT4		= "Outgoing";				// пропуск этой записи
static std::string SIP_COMMANDS_EXT5		= "FMPR";					// пропуск этой записи (эта запись на исходящий звонок)
static std::string SIP_COMMANDS_FND			= "func-apply-sipheaders|ext-queues";	// поиск по этой строке

static std::string SESSION_QUEUE_RESPONSE	= "asterisk -rx \"queue show %queue\"";
// static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -v \"" + SIP_COMMANDS_EXT1 + "\"" 
// 																							" | grep -v \"" + SIP_COMMANDS_EXT2 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT3 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT4 + "\""
// 																							" | grep -v \"" + SIP_COMMANDS_EXT5 + "\"";
static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMANDS_FND + "\""
																							" | grep -v \"" + SIP_COMMANDS_EXT5 + "\"";





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
	struct ActiveTalkCall 
	{
		std::string phone;			// текущий номер телфеона с которым ведется беседа
		std::string phone_raw;		// текущий номер телфеона с которым ведется беседа (сырой как по aster проходит)
		std::string sip;			// внутренний sip который ведет беседу
		std::string talkTime;		// время развговора  //TODO потом в int переделать	
		std::string callID;			// id звонка
		std::string ivr_callID;		// id звонка в ivr	
	};
	using ActiveTalkCallList = std::vector<ActiveTalkCall>;

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

		 // override IAsteriskData 
		virtual void Start() override;
		virtual void Stop() override;
		virtual void Parsing() override;				// разбор сырых данных
		
	private:
		SP_Queue	&m_queueSession;	// ссылка на очереди
		
		OperatorList	m_listOperators;	
		ActiveTalkCallList	m_listCall;			
		SP_SQL			m_sql;		
		IFile			m_queue;			// запрос информации по текущим очередям
		Log				m_log;
		IFile			m_rawDataTalkCall;

		void CreateListActiveSessionOperators();			// активные операторы в линии
		void CreateListActiveTalkCalls();				// активные звонки в линии

		void CreateActiveOperators(const ecQueueNumber _queue);	// найдем активных операторов в линии
		void CreateOperator(const std::string &_lines, Operator &, ecQueueNumber);	// создание структуры Operator					
		std::string FindSipNumber(const std::string &_lines);	// парсинг нахождения активного sip оператора
		bool FindOnHoldStatus(const std::string &_lines);		// парсинг нахождения статуса onHold
		
		void InsertAndUpdateQueueNumberOperators(); // добавление\обновление номена очереди оператора в БД
		bool IsExistListOperators();		// есть ли данные в m_listOperators
		bool IsExistListOperatorsOnHold();	// есть ли данные в m_listOperators.onHold
		bool IsExistListActiveTalkCalls();	// есть ли данные в m_listCall (т.е. есть ли сейчас какие либо активные звонки по данным астреиска)

		bool IsExistOperatorsQueue();	// существует ли хоть 1 запись в БД sip+очередь
		bool IsExistOperatorsQueue(const std::string &_sip, const std::string &_queue);	// существует ли хоть запись в БД sip+очередь
		void ClearOperatorsQueue();		// очистка таблицы operators_queue
		void CheckOperatorsQueue();		// проверка есть ли оператор еще в очереди
		bool GetActiveQueueOperators(OperatorList &_activeList, std::string &_errorDescription); // активные очереди операторов в БД
		void DeleteOperatorsQueue(const std::string &_sip, const std::string &_queue);	// удаление очереди оператора из БД таблицы operators_queue
		void DeleteOperatorsQueue(const std::string &_sip);								// удаление очереди оператора из БД таблицы operators_queue весь sip
		void InsertOperatorsQueue(const std::string &_sip, const std::string &_queue);	// добавление очереди оператору в БД таблицы operators_queue
	
		bool CreateActiveCall(const std::string &_lines, const std::string &_sipNumber, ActiveTalkCall &_caller); // парсинг и нахождение активного звонка с которым разговаривает оператор
		bool FindActiveCallIvrID(const std::string &_lines, const std::string &_phone, ActiveTalkCall &_caller); // парсинг и нахожднение id_ivr
		bool CheckActiveCall(const ActiveTalkCall &_caller); // проверка корректности структуры звонка

		void UpdateActiveCurrentTalkCalls(); // обновление текущих звонков операторов
		
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

		//доп проверка что нет никаких сейчас разговоров
		bool IxExistManualCheckCurrentTalk(); 
	};
}
using SP_ActiveSession = std::shared_ptr<active_sip::ActiveSession>;

#endif // ACTIVESIP_H
