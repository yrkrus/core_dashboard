//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 04.06.2024				//
//				   SQL запросы	             		//
//													//	
//////////////////////////////////////////////////////
#include <iostream>
#include "Constants.h"
#include <mysql/mysql.h>
#include "IVR.h"
#include "Queue.h"
#include "ActiveSip.h"
#include "RemoteCommands.h"
#include "Log.h"
#include "HouseKeeping.h"
#include <vector>
#include <memory>
#include <unordered_map>


#ifndef SQLREQUEST_H
#define	SQLREQUEST_H


namespace SQL_REQUEST
{
	class SQL
	{
	public:
		SQL();
		~SQL() = default;

		bool isConnectedBD();	// есть ли коннект с БД

		//void query_test();
		//void insertData_test();

		//table IVR
	//	void insertIVR(const char *phone, const char *time, std::string callerid );			// добавление данных в таблицу IVR
	//	bool isExistIVRPhone(const char *phone);											// существует ли такой уже номер в таблице IVR
	//	int getLastIDphone(const char *phone);												// получение последнего ID актуального
	//	void updateIVR(const char *id, const char *phone, const char *time);				// обновление данных в таблице IVR
		


		// table QUEUE
		//void insertQUEUE(const char *queue, const char *phone, const char *time);			// добавление данных в таблицу QUEUE
		//bool isExistQUEUE(const char *queue, const char *phone);							// существует ли такой уже номер в таблице QUEUE
		//void updateQUEUE(const char *id, const char *phone, const char *time);				// обновление данных в таблице QUEUE
		//int getLastIDQUEUE(const char *phone);												// получение последнего ID актуального
		//void updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time);	// обновление данных таблицы QUEUE о том с кем сейчас разговаривает оператор
		//bool isExistQUEUE_SIP(const char *phone);											// существует ли такой номер в таблице QUEUE чтобы добавить sip оператора который с разговор ведет
		//void updateQUEUE_fail(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// обновление данных когда звонок не дождался своей очереди 
		//void updateQUEUE_fail();															// обновление данных когда звонок не дождался своей очереди 
		//void updateIVR_to_queue(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// обновление данных когда у нас звонок из IVR попал в очередь
		//bool isExistQueueAfter20hours();													// проверка есть ли номера которые позвонили после 20:00
		//void updateQUEUE_hash(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// обновление поля hash когда успешно поговорили
		//bool isExistAnsweredAfter20hours();													// проверка есть ли номера которым нужно проставить статус отвечено после того как оператор ушел из линии		
		//void updateAnswered_fail();															// обновление данных когда оператор поговорил и ушел из линии, а звонок все еще находится не обработанным


		// table operators_queue
		//void insertOperatorsQueue(const char *sip, const char *queue);						// добавление sip номере оператора + его очередь
		//void clearOperatorsQueue();															// очистка таблицы operators_queue
		//bool isExistOperatorsQueue_old(const char *sip, const char *queue);						// проверка существует ли такой sip+очередь в БД
		//bool isExistOperatorsQueue_old();														// проверка существует ли хоть 1 запись в БД sip+очередь
		//void checkOperatorsQueue(const std::vector<ACTIVE_SIP_old::Operators_old> &list_operators);	// проверка текущих sip + очередь 
		//void deleteOperatorsQueue(const std::string &sip);											// удаление sip номера оператора из всех очередей
		//void deleteOperatorsQueue(const std::string &sip, const std::string &queue);						// удаление sip номера оператора и конкретной очереди


		// Статистика
		int getIVR_totalCalls();															// сколько всего позвонило на линию IVR
		//int getIVR_totalCalls(const IVR_OLD::CallerID &trunk);									// сколько всего позвонило на линию IVR (поиск по trunk)
		int getQUEUE_Calls(bool answered);													// сколько всего ответило и сколько пропущенных
		int getQUEUE_Calls();																// сколько всего было в очереди

		// RemoteComands
		bool remoteCheckNewCommads();														// проверка если новая команда для  входа\выхода из очереди
		void createListRemoteCommands(std::vector<REMOTE_COMMANDS_old::R_Commands_old> &list);	    // генерация текущиъ найденных команд
		void startRemoteCommand(int id, std::string sip, remote::ecCommand command, int user_id);	// запуск удаленной команды
		void deleteRemoteCommand(int id);													// удаление успешно выполненной команды
		void updateStatusOperators(int user_id, remote::ecStatusOperator status);	// обновление текущего статуса оператора


		// Логирование 
		void addLog(remote::ecCommand command, int base_id);    // создание лога в БД
		
		
		// Housekeeping
		void execTaskQueue();																// выполнение задачи очистка таблицы queue
		void execTaskLogging();																// выполнение задачи очистки таблицы Logging
		void execTaskIvr();																	// выполнение задачи очистки таблицы Ivr
		void execTaskOnHold();																// выполнение задачи очистки таблицы operators_onhold
		void execTaskSmsSending();															// выполнение задачи очистки таблицы sms_sending	
		bool insertDataTaskQueue(HOUSEKEEPING::Queue_old &queue);								// вставка данных для таблицы history_queue			
		bool deleteDataTaskQueue(int ID);													// удаление данных из таблицы queue
		bool insertDataTaskLogging(HOUSEKEEPING::Logging &logging);							// вставка данных для таблицы history_logging
		bool deleteDataTaskLogging(int ID);													// удаление данных из таблицы logging
		bool insertDataTaskIvr(HOUSEKEEPING::IVR_ &ivr);										// вставка данных из талицы history_ivr
		bool deleteDataTaskIvr(int ID);														// удаление данных из таблицы ivr 
		bool insertDataTaskOnHold(HOUSEKEEPING::OnHold &onHold);							// вставка данных для таблицы history_onhold
		bool deleteDataTaskOnHold(int ID);													// удаление данных из таблицы operatorsd_onhold
		bool insertDataTaskSmsSending(const HOUSEKEEPING::SmsSending &_smsSending);		    // вставка данных для таблицы history_sms_sending
		bool deleteDataTaskSmsSending(int _id);	  											// удаление данных из таблицы sms_sending


		// table operators_ohhold
		//void updateOperatorsOnHold(ACTIVE_SIP_old::Parsing_old *list);						// обнление данных о статусе оператора OnHold
		//std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> createOnHoldSip();								    // создание списка с sip оператоарми которые находятся в статусе OnHold по БД
		//void addOperatorsOnHold(const std::string &sip, const std::string &phone);				// добавление sip номера орератора в таблицу operators_onhold
		//void disableOperatorsOnHold(const ACTIVE_SIP_old::OnHold_old &onhold);						// обновление данных что sip закончил быть в статусе onHold 
		//bool isExistOnHold(const std::string &sip, std::string hash);							// проверка существует ли такой уже onHold (что бы обезопаситься от дубля)											


	private:
		MYSQL mysql;
		void createMySQLConnect(MYSQL &mysql);												// подключаемся к БД MySQL
	};	

};


#endif //SQLREQUEST_H