#include "InternalFunction.h"
#include "Constants.h"
#include "IVR.h"
#include "Queue.h"
#include "ActiveSip.h"
#include "SQLRequest.h"
#include "Log.h"
#include <cmath>
#include <time.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>


//#ifdef _WIN32
//#include "mysql/mysql.h"
//#elif __linux__
#include <mysql/mysql.h>
//#endif


// парсинг номера телефона в нормальный вид
std::string phoneParsing(std::string &phone)
{
	if (phone.length() < 6)
	{
		return "null";	// для номеров которые не сотовые и не городские
	}
	else if (phone.length() == 10)
	{
		return "+7" + phone;
	}
	else if (phone.length() == 11)
	{
		return "+" + phone;
	}
	else
	{
		return "null";	// ничего не нашли возращаем null
	}
};



// создать + получить текущий IVR
void getIVR()
{   
	if (!CONSTANTS::DEBUG_MODE) {
		system(CONSTANTS::cIVRResponse.c_str());
	};	  

    // разбираем
    IVR::Parsing ivr(CONSTANTS::cIVRName.c_str());
	if (ivr.isExistList()) {
		ivr.show();		
		ivr.insertData();
	}
}

// создать + получить текущую очередь
void getQueue(void)
{
	if (!CONSTANTS::DEBUG_MODE)	{
		system(CONSTANTS::cQueueResponse.c_str());
	}

    QUEUE::Parsing queue(CONSTANTS::cQueueName.c_str());
    
	if (queue.isExistList()) {
		queue.show();
		queue.insertData();
	}
	else {
		// проверим время вдруг кто то позвонил после 20:00:00 и был в очереди, тогда надо пройтись 1 раз по БД
		if (queue.isExistQueueAfter20hours()) {
			// есть потеряшки, обновляем данные по ним
			queue.updateQueueAfter20hours();			
		}
		// проверим есть ли не про hash'нные номера, когда оператор уже закончил разговор и ушел из линии
		if (queue.isExistAnsweredAfter20hours()) {
			queue.updateAnsweredAfter20hours();
		}

	}
}

// создать + получить кто с кем разговаривает
void getActiveSip(void)
{
	if (!CONSTANTS::DEBUG_MODE)	{
		system(CONSTANTS::cActiveSipResponse.c_str());
	}

    ACTIVE_SIP::Parsing sip(CONSTANTS::cActiveSipName.c_str());
	if (sip.isExistList()) { 
		sip.show(); 
		sip.updateData();
	}	
}

// получение номера очереди
std::string getNumberQueue(CONSTANTS::AsteriskQueue queue)
{
	switch (queue)
	{
		case CONSTANTS::main: {
			return "5000";
			break;
		}			
		case CONSTANTS::lukoil: {
			return "5050";
			break;
		}			
		default: {
			return "5000";
			break;
		}			
	}
}

// перевод общего кол-ва секунда в 00:00:00 формат
std::string getTalkTime(std::string talk)
{
	const unsigned int daysCount	= 24 * 3600;
	const unsigned short hourCount	= 3600;
	const unsigned short minCount	= 60;

	std::string resultat;

	int talk_time = std::stoi(talk);

	int days, hour, min,sec;
	std::string shour, smin, ssec;

	days = hour = min = sec = 0;

	days = static_cast<int>(std::floor((talk_time / daysCount)));		
	hour = static_cast<int>(std::floor(((talk_time - (days * daysCount)) / hourCount)));	
	min = static_cast<int>(std::floor(((talk_time - ((days * daysCount) + (hour * hourCount) )) / minCount)));
	sec = static_cast<int>(std::floor((talk_time - ((days * daysCount) + (hour * hourCount) + (min * minCount)))));
		
	(hour < 10) ? shour = "0" + std::to_string(hour) : shour = std::to_string(hour);
	(min < 10) ? smin = "0" + std::to_string(min) : smin = std::to_string(min);
	(sec < 10) ? ssec = "0" + std::to_string(sec) : ssec = std::to_string(sec);
	
	resultat = shour + ":" + smin + ":" + ssec;	
	return ((days == 0) ? resultat : resultat += std::to_string(days)+"d "+ resultat);	
}

// текущее время 
std::string getCurrentDateTime()
{	
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);		

	//формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }

	std::string hour = std::to_string(now_tm->tm_hour);
	if (hour.length() == 1) { hour = "0" + hour; }

	std::string min = std::to_string(now_tm->tm_min);
	if (min.length() == 1) { min = "0" + min; }

	std::string sec = std::to_string(now_tm->tm_sec);
	if (sec.length() == 1) { sec = "0" + sec; }
		
	return year + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
}

// текущее начало дня
std::string getCurrentStartDay()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	//формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }	

	return year + "-" + mon + "-" + day + " 00:00:00" ;
}

// текущее время - 2 минута 
std::string getCurrentDateTimeAfterMinutes(int minutes)
{
	auto now = std::chrono::system_clock::now();
	auto minute = std::chrono::minutes(minutes);

	std::time_t now_c = std::chrono::system_clock::to_time_t(now-minute);
	struct std::tm *now_tm = std::localtime(&now_c);

	//формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }	

	std::string hour = std::to_string(now_tm->tm_hour);
	if (hour.length() == 1) { hour = "0" + hour; }

	std::string min = std::to_string(now_tm->tm_min);
	if (min.length() == 1) { min = "0" + min; }

	std::string sec = std::to_string(now_tm->tm_sec);
	if (sec.length() == 1) { sec = "0" + sec; }

	return year + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
}

// текущее время после 20:00 
std::string getCurrentDateTimeAfter20hours()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	//формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }

	return year + "-" + mon + "-" + day + " 20:00:00";
}

// показ версии ядра
void showVersionCore()
{
	std::ostringstream buffer;
	buffer << "==========================================================\n";
	
	if (CONSTANTS::DEBUG_MODE) // DEBUG
	{
		buffer << "###DEBUG### " + CONSTANTS::core_version << "\n";
	}
	else {					  // NO DEBUG
		buffer << "\t" << CONSTANTS::core_version << "\n";
	}	
	
	buffer << "==========================================================\n";
	std::cout << buffer.str();
}

// статистика // пока без класса, может потом в отдельный класс сделать
void getStatistics()
{
	SQL_REQUEST::SQL base;

	std::ostringstream buffer;

	int totalcalls				= base.getIVR_totalCalls();
	int totalqueue				= base.getQUEUE_Calls();
	int totalqueue_answered		= base.getQUEUE_Calls(true);
	int totalqueue_no_answered	= base.getQUEUE_Calls(false);

	
	int totalComagic = base.getIVR_totalCalls(IVR::comagic);
	int total220220	 = base.getIVR_totalCalls(IVR::domru_220220);
	int total220000  = base.getIVR_totalCalls(IVR::domru_220000);
	int totalSts	 = base.getIVR_totalCalls(IVR::sts);
	int totalNull	 = base.getIVR_totalCalls(IVR::null_caller);


	buffer << "\n\tStatistics QUEUE\n";

	buffer << "total calls IVR\t\t\t" << totalcalls<<"\n";
	buffer << "total queue \t\t\t" << totalqueue << "\n";

	buffer << "total queue answered\t\t" << totalqueue_answered << "\n";
	buffer << "total queue no answered\t\t" << totalqueue_no_answered << "\n";
	
	buffer << "response rate\t\t\t" << std::setprecision(4) << (totalqueue != 0  ? (totalqueue_answered * 100 / static_cast<double>(totalqueue)) : 0) << "%\n";
	buffer << "no answered rate\t\t" << std::setprecision(4) << (totalqueue_answered != 0 ? (totalqueue_no_answered * 100 / static_cast<double>(totalqueue_answered)) : 0) << "%\n";

	buffer << "\n\n\tStatistics CALLERS\n";
	buffer << "220-220 \t\t\t" << total220220 << "(" << std::setprecision(4) << (totalcalls != 0 ? (total220220 * 100 / static_cast<double>(totalcalls)) : 0) << "%)\n";
	buffer << "Comagic \t\t\t"<< totalComagic <<"("<< std::setprecision(4) << (totalcalls != 0 ? (totalComagic * 100 / static_cast<double>(totalcalls)) : 0) << "%)\n";
	buffer << "Sts \t\t\t\t" << totalSts << "(" << std::setprecision(4) << (totalcalls != 0 ? (totalSts * 100 / static_cast<double>(totalcalls)) : 0) << "%)\n";
	buffer << "220-000 \t\t\t" << total220000 << "(" << std::setprecision(4) << (totalcalls != 0 ? (total220000 * 100 / static_cast<double>(totalcalls)) : 0) << "%)\n";
	buffer << "null callers \t\t\t" << totalNull << "(" << std::setprecision(4) << (totalcalls != 0 ? (totalNull * 100 / static_cast<double>(totalcalls)) : 0) << "%)\n";

	std::cout << buffer.str();
}

// отображжение инфо что не возможно подключиться к бд
void showErrorBD(const std::string str)
{
	std::cerr << str << " -> Error: can't connect to database " << CONSTANTS::cHOST << ":" << CONSTANTS::cBD << "\n";
}

// отображение инфо что пошла какая то ошибка
void showErrorBD(const std::string str, MYSQL *mysql)
{
	std::cerr << str <<" " << mysql_error(mysql) <<"\n";
}

// преобразование текущей удаленной комады из int -> REMOTE_COMMANDS::Command 
LOG::Log getRemoteCommand(int command)
{
	return static_cast<LOG::Log>(command);
}

// преобразование текущей удаленной комады из LOG::Log -> int
int getRemoteCommand(LOG::Log command)
{
	return static_cast<int>(command);
}

// преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int
int getStatusOperators(REMOTE_COMMANDS::StatusOperators status)
{
	return static_cast<int>(status);
}

// проверка успешно ли выполнили удаленную команду
bool remoteCommandChekedExecution(LOG::Log command)
{	
	
	std::ifstream fileRemoteCommand;
	fileRemoteCommand.open(CONSTANTS::cRemoteCommandName);

	// не удается открыть файл
	if (!fileRemoteCommand.is_open()) return false;

	// находим и разбираем команду 
	std::string line;
	

	while (std::getline(fileRemoteCommand, line)) {		
		
		// какую именно команду ищем
		if (command == LOG::Log::Log_add_queue_5000 ||
			command == LOG::Log::Log_add_queue_5050 ||
			command == LOG::Log::Log_add_queue_5000_5050)
		{
			if ((line.find("Added") != std::string::npos) || (line.find("Already there") != std::string::npos))
			{
				fileRemoteCommand.close();
				return true;				
			}
		}
		else if (command == LOG::Log::Log_del_queue_5000 ||
				 command == LOG::Log::Log_del_queue_5050 ||
				 command == LOG::Log::Log_del_queue_5000_5050 ||
				 command == LOG::Log::Log_home ||
				 command == LOG::Log::Log_exodus ||
				 command == LOG::Log::Log_break ||
				 command == LOG::Log::Log_dinner ||
				 command == LOG::Log::Log_postvyzov ||
				 command == LOG::Log::Log_studies ||
				 command == LOG::Log::Log_IT ||
				 command == LOG::Log::Log_transfer ||
				 command == LOG::Log::Log_reserve) 
		{
			if ((line.find("Removed") != std::string::npos) || (line.find("Not there") != std::string::npos))
			{
				fileRemoteCommand.close();
				return true;				
			}
		}		
		
	}

	fileRemoteCommand.close();
	return false;	
}


bool to_bool(std::string str)
{
	return ((str == "true") ? true : false);
}










