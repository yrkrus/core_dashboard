  
//	      парсинг звонков попадающих в Очередь		

#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "Log.h"
#include "IAsteriskData.h"
#include "ISQLConnect.h"


static std::string QUEUE_COMMANDS		= "Queue|to-atsaero5005";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";

enum class EQueueNumber // ВАЖНО в методе bool Status::CreateCommand используется for 1..2 
{
	Unknown = 0,
	e5000,	
	e5050,
	e5000_e5050,	// сочетение 5000+5050 
	e5005,			// очередь для бабы железной
};
using QueueList = std::vector<EQueueNumber>;

class Queue;
using SP_Queue = std::shared_ptr<Queue>;


class Queue : public IAsteriskData
{
public:	
	struct QueueCalls
	{
		std::string phone	= "null";					// текущий номер телефона который в очереди сейчас
		std::string waiting = "null";					// время в (сек) которое сейчас в очереди находится
		EQueueNumber queue	= EQueueNumber::Unknown;	// номер очереди
	
		inline bool check() const noexcept
		{
			// если в phone или waiting есть подстрока "null" 
			// или callerID == Unknown — сразу false
			if (phone.find("null")		!= std::string::npos ||
				waiting.find("null")	!= std::string::npos ||
				queue					== EQueueNumber::Unknown)
			{				 
				return false;
			}

			return true;
		}
	};
	using QueueCallsList = std::vector<QueueCalls>;
	
	struct CallsInBase	// структура из БД
	{
		std::string id			= "-1";
		std::string phone		= "null";
		std::string date_time	= "null";
		size_t hash				= 0;
	};
	using CallsInBaseList = std::vector<CallsInBase>;

	
	Queue();
	~Queue() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// разбор сырых данных

	void UpdateCallSuccess();							// обновление данных когда нет активных операторов на линии
	private:
	QueueCallsList		m_listQueue;
	SP_SQL				m_sql;
	Log					m_log;
	
	void UpdateCalls(const QueueCallsList &_callList);  			// обновление звонков

	bool FindQueueCallers();									// поиск текущих активных звонков

	bool CreateQueueCallers(const std::string&, QueueCalls&);
	bool CheckCallers(const QueueCalls&);						// проверка корреткности стуктуры звонка

	bool IsExistQueueCalls();									// есть ли звонки в памяти

	void InsertQueueCalls();							// добавление данных в БД
	void InsertCall(const QueueCalls &_call);			// добавление нового звонка
	void InsertCallVirtualOperator(const QueueCalls &_call);	// добавление нового звонка (виртуальный оператор лиза)
	bool UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription); // обновление существующего звонка
	bool UpdateCallVirualOperator(int _id, const QueueCalls &_call, std::string &_errorDescription); // обновление существующего звонка (виртуальный оператор)
	void UpdateCallFail(const QueueCallsList &_calls);	// обновление данных если звонок был в очереди, но не дождался ответа от оператора
	//void UpdateCallFail();								// обновление данных если звонок был в очереди, но не дождался ответа от оператора
	void UpdateCallIvr(const QueueCallsList &_calls);	// обновление данных когда у нас звонок из IVR попал в очередь или на виртуального оператора
	void UpdateCallIvrToQueue(const QueueCallsList &_calls);	// звонок из IVR попал в очередь
	void UpdateCallIvrToVirtualOperator (const QueueCallsList &_calls);	// звонок из IVR попал на виртуального оператора
	
	void UpdateCallSuccess(const QueueCallsList &_calls);				// обновление данных когда разговор успешно состоялся
	void UpdateCallSuccessRealOperator(const QueueCallsList &_calls);	// разговор успешно состоялся реальный оператор
	void UpdateCallSuccessVirtualOperator(const QueueCallsList &_calls);// разговор успешно состоялся виртуальный оператор
	
	bool IsExistCall(EQueueNumber _queue, const std::string &_phone);	// есть ли уже такой номер в БД
	bool IsExistCallVirtualOperator(EQueueNumber _queue, const std::string &_phone);	// есть ли уже такой номер в БД (виртуальный оператор)

	int GetLastQueueCallId(const std::string &_phone);					// id записи по БД о звонке
	int GetLastQueueVirtualOperatorCallId(const std::string &_phone);	// id записи по БД о звонке(виртуальный оператор)

	bool GetCallsInBase(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription); // получение записей из БД 
	bool GetCallsInBase(CallsInBaseList &_vcalls, std::string &_errorDescription);						// получение записей из БД 
	bool GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription); // получение записей из БД (виртуальный оператор)
	bool GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, std::string &_errorDescription);	// получение записей из БД (виртуальный оператор)

	//bool IsExistCallAfter20Hours(std::string &_errorDescription);			// есть ли звонок после 20:00
	//void UpdateCallsAfter20hours();											// есть потеряшки которые звонили после 20:00, обновить их
	bool IsExistAnyAnsweredCall();					// есть ли не про hash'нные номера, когда оператор уже закончил разговор и ушел из линии
	void UpdateAllAnyAnsweredCalls();				// есть не про hash'нные номера обновляем их (ВСЕ!)

};


#endif //QUEUE_H
