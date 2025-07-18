#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

#include <string>
#include <list>
#include <unordered_map>
#include "Constants.h"
#include "SQLRequest.h"
#include "Queue.h"
#include "RemoteCommands.h"
#include "Log.h"
#include <mysql/mysql.h>
#include <map>

namespace utils 
{
	
	//typedef std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> SP_OnHold;
	typedef std::vector<ACTIVE_SIP_old::Operators_old> Operators_old;
	//typedef std::shared_ptr<std::map<std::string, std::string>> SP_NewOnHoldOperators;

	std::string StringFormat(const char *format, ...);
	std::string PhoneParsing(std::string &phone);					// парсинг номера телефона в нормальный вид



	
	//void getIVR();												    // создать + получить текущий IVR
	//void getQueue(void);											// создать + получить текущую очередь
	//void getActiveSip(void);										// создать + получить кто с кем разговаривает
	//std::string getNumberQueue(CONSTANTS::AsteriskQueue queue);		// получение номера очереди
	std::string getTalkTime(std::string talk);						// перевод временни из сек -> 00:00:00

	// функции работы со временем (формат год-месяц-день 00:00:00 )
	std::string GetCurrentDateTime();							// текущее время 
	std::string GetCurrentStartDay();							// текущее начало дня
	std::string GetCurrentDateTimeAfterMinutes(int minutes);	// текущее время - (МИНУС) указанная минута 
	//std::string getCurrentDateTimeAfter20hours();				// текущее время после 20:00 

	void showVersionCore(unsigned int iter);					// показ версии ядра

	// статистика // пока без класса, может потом в отдельный класс сделать
	void getStatistics();

	void showErrorBD(const std::string str); // отображжение инфо что не возможно подключиться к бд
	void showErrorBD(const std::string str, MYSQL *mysql); // отображение инфо что пошла какая то ошибка

	ECommand getRemoteCommand(int command);	// преобразование текущей удаленной комады из int -> LOG::Log
	int getRemoteCommand(ECommand command); // преобразование текущей удаленной комады из LOG::Log -> int
	int getStatusOperators(EStatus status); // преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int
	//bool isExistNewOnHoldOperators(const SP_OnHold &onHold, const Operators &operators); // проверка есть ли разница между onHold по БД и active_sip->onHold
	//SP_NewOnHoldOperators createNewOnHoldOperators(const SP_OnHold &onHold, const Operators &operators);			// создание списка с новыми операторами в OnHold


	//bool remoteCommandChekedExecution(remote::ECommand command);	// проверка успешно ли выполнили удаленную команду

	bool to_bool(const std::string &str);						// конвертер из std::string -> bool
	std::string to_string(bool value);					// конвертер из bool -> std::string 

	size_t string_to_size_t(const std::string &str);	// конвертер std::string->size_t

	void showHelpInfo();						// отображение хелп справки, когда не корректную команду ввесли
	

	//@param &_replacmentResponse - изменяемая строка
	//@param &_find - параметр который ищем
	//@param &_repl - параметр НА которой будем заменять
	void ReplaceResponseStatus(std::string &_replacmentResponse, const std::string &_find, const std::string &_repl);
}
	

#endif //INTERNALFUNCTION_H