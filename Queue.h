  
//	      парсинг звонков попадающих в Очередь		

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef QUEUE_H
#define QUEUE_H

static std::string QUEUE_COMMANDS		= "Queue";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";


class Queue : public IAsteriskData
{
public:
	
	enum class ecQueueNumber 
	{
		eUnknown = 0,
		e5000,
		e5050,
		e5100,
	};

	struct QueueCalls
	{
		std::string phone	= "null";					// текущий номер телефона который в очереди сейчас
		std::string waiting = "null";					// время в (сек) которое сейчас в очереди находится
		ecQueueNumber queue	= ecQueueNumber::eUnknown;	// номер очереди
	};

	struct CallsInBase	// структура из БД
	{
		std::string id			= "- 1";
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

private:
	std::vector<QueueCalls>	m_listQueue;
	SP_SQL					m_sql;
	
	
	bool FindQueueCallers();									// поиск текущих активных звонков

	bool CreateQueueCallers(const std::string&, QueueCalls&);
	bool CheckCallers(const QueueCalls&);							// проверка корреткности стуктуры звонка

	bool IsExistQueueCalls();										// есть ли звонки в памяти

	ecQueueNumber StringToEnum(const std::string &_str);
	std::string EnumToString(ecQueueNumber _number);

	void InsertQueueCalls();													// добавление данных в БД
	void InsertCall(const QueueCalls &_call);									// добавление нового звонка
	bool UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription); // обновление существующего звонка
	void UpdateCallFail(const std::vector<QueueCalls> &_calls);					// обновление данных если звонок был в очереди, но не дождался ответа от оператора
	void UpdateCallFail();														// обновление данных если звонок был в очереди, но не дождался ответа от оператора
	void UpdateCallToIVR(const std::vector<QueueCalls> &_calls);				// обновление данных когда у нас звонок из IVR попал в очередь
	void UpdateCallSuccess(const std::vector<QueueCalls> &_calls);				// обновление данных когда разговор успешно состоялся

	bool IsExistCall(ecQueueNumber _queue, const std::string &_phone);			// есть ли уже такой номер в БД
	int GetLastQueueCallId(const std::string &_phone);							// id записи по БД о звонке

	bool GetCallsInBase(CallsInBaseList &_vcalls, const std::vector<QueueCalls> &_queueCalls, std::string &_errorDescription); // получение записей из БД 
	bool GetCallsInBase(CallsInBaseList &_vcalls, std::string &_errorDescription);						// получение записей из БД 

	bool IsExistCallAfter20Hours(std::string &_errorDescription);			// есть ли звонок после 20:00
	void UpdateCallsAfter20hours();											// есть потеряшки которые звонили после 20:00, обновить их

	bool IsExistCallAnsweredAfter20hours(std::string &_errorDescription);	// есть ли не про hash'нные номера, когда оператор уже закончил разговор и ушел из линии
	void UpdateCallAnsweredAfter20hours();									// есть не про hash'нные номера обновляем их

	


};



namespace QUEUE_OLD
{
	enum Currentfind
	{
		phone_find,
		waiting_find,
		queue_find,
	};

	struct Pacients_old
	{
		std::string phone	{ "null" };		// текущий номер телефона который в очереди сейчас
		std::string waiting { "null" };		// время в (сек) которое сейчас в очереди находится
		std::string queue	{ "null" };		// номер очереди
	};

	class Parsing
	{
	public:
		Parsing(const char *fileQueue);
		~Parsing() = default;

		bool isExistList();					// есть ли очередь	
		void show(bool silent = false);
		
		void insertData();					// добавление данных в БД
		bool isExistQueueAfter20hours();	// проверка есть ли не отвеченные записи после 20:00
		void updateQueueAfter20hours();		// обновление данных если звонок пришел того как нет активных операторов на линии	
		bool isExistAnsweredAfter20hours(); // проверка если ли номера по которым закончили разгвоаривать, но не успели обработать
		void updateAnsweredAfter20hours();	// обновление данных когда закончили разговаривать и ушли с линии
		

	private:
		std::string findParsing(std::string str, QUEUE_OLD::Currentfind find); // парсинг  
		std::vector<QUEUE_OLD::Pacients_old> pacient_list;
	};


	struct BD 
	{
		std::string id;
		std::string phone;
		std::string date_time;
		size_t hash{0};
	};

	class QueueBD_old 
	{
		public:
			QueueBD_old()	= default;
			~QueueBD_old()	= default;
			std::vector<QUEUE_OLD::BD> list;
	};
}

#endif //QUEUE_H
