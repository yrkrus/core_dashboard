//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//		внутренние функции не вошедшие в классы 	//
//													//	
//////////////////////////////////////////////////////

#include <string>
#include "Constants.h"
#include "SQLRequest.h"
#include "Queue.h"
#include "RemoteCommands.h"
#include "Log.h"
#include <mysql/mysql.h>


#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

std::string phoneParsing(std::string &phone);					// парсинг номера телефона в нормальный вид
void getIVR();												    // создать + получить текущий IVR
void getQueue(void);											// создать + получить текущую очередь
void getActiveSip(void);										// создать + получить кто с кем разговаривает
std::string getNumberQueue(CONSTANTS::AsteriskQueue queue);		// получение номера очереди
std::string getTalkTime(std::string talk);						// перевод временни из сек -> 00:00:00

// функции работы со временем (формат год-мес€ц-день 00:00:00 )
std::string getCurrentDateTime();							// текущее врем€ 
std::string getCurrentStartDay();							// текущее начало дн€
std::string getCurrentDateTimeAfterMinutes(int minutes);	// текущее врем€ -(ћ»Ќ”—) указанна€ минута 
std::string getCurrentDateTimeAfter20hours();				// текущее врем€ после 20:00 

void showVersionCore();										// показ версии €дра

// статистика // пока без класса, может потом в отдельный класс сделать
void getStatistics();

void showErrorBD(const std::string str); // отображжение инфо что не возможно подключитьс€ к бд
void showErrorBD(const std::string str, MYSQL *mysql); // отображение инфо что пошла кака€ то ошибка

LOG::Log getRemoteCommand(int command);	// преобразование текущей удаленной комады из int -> LOG::Log
int getRemoteCommand(LOG::Log command); // преобразование текущей удаленной комады из LOG::Log -> int
int getStatusOperators(REMOTE_COMMANDS::StatusOperators status); // преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int



bool remoteCommandChekedExecution(LOG::Log command);	// проверка успешно ли выполнили удаленную команду

bool to_bool(std::string str);						// конвертер из std::string -> bool



#endif //INTERNALFUNCTION_H