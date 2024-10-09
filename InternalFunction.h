//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//		внутренние функции не вошедшие в классы 	//
//													//	
//////////////////////////////////////////////////////

#include <string>
#include <list>
#include <unordered_map>
#include "Constants.h"
#include "SQLRequest.h"
#include "Queue.h"
#include "RemoteCommands.h"
#include "Log.h"
#include <mysql/mysql.h>


#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

namespace INTERNALFUNCTION {
	
	typedef std::vector<ACTIVE_SIP::OnHold> OnHold;
	typedef std::vector<ACTIVE_SIP::Operators> Operators;

	std::string phoneParsing(std::string &phone);					// парсинг номера телефона в нормальный вид
	void getIVR();												    // создать + получить текущий IVR
	void getQueue(void);											// создать + получить текущую очередь
	void getActiveSip(void);										// создать + получить кто с кем разговаривает
	std::string getNumberQueue(CONSTANTS::AsteriskQueue queue);		// получение номера очереди
	std::string getTalkTime(std::string talk);						// перевод временни из сек -> 00:00:00

	// функции работы со временем (формат год-месяц-день 00:00:00 )
	std::string getCurrentDateTime();							// текущее время 
	std::string getCurrentStartDay();							// текущее начало дня
	std::string getCurrentDateTimeAfterMinutes(int minutes);	// текущее время -(МИНУС) указанная минута 
	std::string getCurrentDateTimeAfter20hours();				// текущее время после 20:00 

	void showVersionCore(unsigned int iter);					// показ версии ядра

	// статистика // пока без класса, может потом в отдельный класс сделать
	void getStatistics();

	void showErrorBD(const std::string str); // отображжение инфо что не возможно подключиться к бд
	void showErrorBD(const std::string str, MYSQL *mysql); // отображение инфо что пошла какая то ошибка

	LOG::Log getRemoteCommand(int command);	// преобразование текущей удаленной комады из int -> LOG::Log
	int getRemoteCommand(LOG::Log command); // преобразование текущей удаленной комады из LOG::Log -> int
	int getStatusOperators(REMOTE_COMMANDS::StatusOperators status); // преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int
	bool isExistNewOnHoldOperators(const OnHold *onHold, const Operators &operators); // проверка есть ли разница между onHold по БД и active_sip->onHold
	std::unordered_map <std::string, std::string> *createNewOnHoldOperators(const OnHold &onHold, const Operators &operators);			// создание списка с новыми операторами в OnHold


	bool remoteCommandChekedExecution(LOG::Log command);	// проверка успешно ли выполнили удаленную команду

	bool to_bool(std::string str);						// конвертер из std::string -> bool
	std::string to_string(bool value);					// конвертер из bool -> std::string 

	size_t string_to_size_t(const std::string &str);	// конвертер std::string->size_t
}
	

#endif //INTERNALFUNCTION_H