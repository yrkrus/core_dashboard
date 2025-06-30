  
//	      парсинг звонков попадающих в Очередь		

#include <string>
#include <vector>
//#include <iostream>
#include <sstream>
#include <map>

#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef QUEUE_H
#define QUEUE_H

static std::string QUEUE_COMMANDS		= "Queue|to-atsaero5005";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";

enum class ecQueueNumber
{
	Unknown = 0,
	e5000,
	e5005,	// очередь для бабы железной
	e5050,
	e5000_e5050,	// сочетение 5000+5050 (испотльзуется почти никогда)
};
class Queue;
typedef std::shared_ptr<Queue> SP_Queue;

class Queue : public IAsteriskData
{
public:	
	struct QueueCalls
	{
		std::string phone	= "null";					// текущий номер телефона который в очереди сейчас
		std::string waiting = "null";					// время в (сек) которое сейчас в очереди находится
		ecQueueNumber queue	= ecQueueNumber::Unknown;	// номер очереди
	};
	typedef std::vector<QueueCalls> QueueCallsList;

	struct CallsInBase	// структура из БД
	{
		std::string id			= "-1";
		std::string phone		= "null";
		std::string date_time	= "null";
		size_t hash				= 0;
	};
	typedef std::vector<CallsInBase> CallsInBaseList;
	
	Queue();
	~Queue() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// разбор сырых данных

	void UpdateCallSuccess();							// обновление данных когда разговор успешно состоялся (все звонки)
	private:
	QueueCallsList		m_listQueue;
	SP_SQL				m_sql;
	
	
	void UpdateCalls();									// обновление звонков

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
	
	bool IsExistCall(ecQueueNumber _queue, const std::string &_phone);	// есть ли уже такой номер в БД
	bool IsExistCallVirtualOperator(ecQueueNumber _queue, const std::string &_phone);	// есть ли уже такой номер в БД (виртуальный оператор)

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

template<typename T>
T StringToEnum(const std::string&);

template<>
inline ecQueueNumber StringToEnum<ecQueueNumber>(const std::string &_str)
{
	if (_str.find("5000") != std::string::npos)		return ecQueueNumber::e5000;
	if (_str.find("5005") != std::string::npos)		return ecQueueNumber::e5005;
	if (_str.find("5050") != std::string::npos)		return ecQueueNumber::e5050;
	if (_str.find("5000 и 5050") != std::string::npos)		return ecQueueNumber::e5000_e5050;

	return ecQueueNumber::Unknown;
}

template<typename T>
std::string EnumToString(T);

template<>
inline std::string EnumToString<ecQueueNumber>(ecQueueNumber _number)
{
	static std::map<ecQueueNumber, std::string> queueNumber =
	{
		{ecQueueNumber::Unknown,	"Unknown"},
		{ecQueueNumber::e5000,		"5000"},
		{ecQueueNumber::e5005,		"5005"},
		{ecQueueNumber::e5050,		"5050"},
		{ecQueueNumber::e5000_e5050,"5000 и 5050"},
	};

	auto it = queueNumber.find(_number);
	if (it != queueNumber.end())
	{
		return it->second;
	}
	return "Unknown";
}



//namespace QUEUE_OLD
//{
//	enum Currentfind
//	{
//		phone_find,
//		waiting_find,
//		queue_find,
//	};
//
//	struct Pacients_old
//	{
//		std::string phone	{ "null" };		// текущий номер телефона который в очереди сейчас
//		std::string waiting { "null" };		// время в (сек) которое сейчас в очереди находится
//		std::string queue	{ "null" };		// номер очереди
//	};
//
//	class Parsing
//	{
//	public:
//		Parsing(const char *fileQueue);
//		~Parsing() = default;
//
//		bool isExistList();					// есть ли очередь	
//		void show(bool silent = false);
//		
//		void insertData();					// добавление данных в БД
//		bool isExistQueueAfter20hours();	// проверка есть ли не отвеченные записи после 20:00
//		void updateQueueAfter20hours();		// обновление данных если звонок пришел того как нет активных операторов на линии	
//		bool isExistAnsweredAfter20hours(); // проверка если ли номера по которым закончили разгвоаривать, но не успели обработать
//		void updateAnsweredAfter20hours();	// обновление данных когда закончили разговаривать и ушли с линии
//		
//
//	private:
//		std::string findParsing(std::string str, QUEUE_OLD::Currentfind find); // парсинг  
//		std::vector<QUEUE_OLD::Pacients_old> pacient_list;
//	};
//
//
//	struct BD 
//	{
//		std::string id;
//		std::string phone;
//		std::string date_time;
//		size_t hash{0};
//	};
//
//	class QueueBD_old 
//	{
//		public:
//			QueueBD_old()	= default;
//			~QueueBD_old()	= default;
//			std::vector<QUEUE_OLD::BD> list;
//	};
//}

#endif //QUEUE_H
