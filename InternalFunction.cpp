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


std::string INTERNALFUNCTION::StringFormat(const char *format, ...)
{
	// ������� ������ ostringstream ��� �������������� ������
	std::ostringstream oss;

	// �������������� ������ ����������
	va_list args;
	va_start(args, format);

	while (*format)
	{
		if (*format == '%')
		{
			format++;
			switch (*format)
			{
			case 'd':
			{  // ��� ����� �����
				int i = va_arg(args, int);
				oss << i;
				break;
			}
			case 'f':
			{ // ��� ����� � ��������� ������
				double d = va_arg(args, double);
				oss << d;
				break;
			}
			case 's':
			{ // ��� �����
				char *s = va_arg(args, char *);
				oss << s;
				break;
			}
			default: // ��������� ����������� ��������������
				oss << '%' << *format;
				break;
			}
		}
		else
		{
			oss << *format; // ������ ��������� ������
		}
		format++;
	}

	va_end(args);

	return oss.str();
}

// ������� ������ �������� � ���������� ���
std::string INTERNALFUNCTION::phoneParsing(std::string &phone)
{
	if (phone.length() < 6)
	{
		return "null";	// ��� ������� ������� �� ������� � �� ���������
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
		return "null";	// ������ �� ����� ��������� null
	}
};



// ������� + �������� ������� IVR
void INTERNALFUNCTION::getIVR()
{   
	if (!CONSTANTS::DEBUG_MODE) {
		system(CONSTANTS::cIVRResponse.c_str());
	};	  

    // ���������
    IVR::Parsing ivr(CONSTANTS::cIVRName.c_str());
	if (ivr.isExistList()) {
		ivr.show();		
		ivr.insertData();
	}
}

// ������� + �������� ������� �������
void INTERNALFUNCTION::getQueue(void)
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
		// �������� ����� ����� ��� �� �������� ����� 20:00:00 � ��� � �������, ����� ���� �������� 1 ��� �� ��
		if (queue.isExistQueueAfter20hours()) {
			// ���� ���������, ��������� ������ �� ���
			queue.updateQueueAfter20hours();			
		}
		// �������� ���� �� �� ��� hash'���� ������, ����� �������� ��� �������� �������� � ���� �� �����
		if (queue.isExistAnsweredAfter20hours()) {
			queue.updateAnsweredAfter20hours();
		}

	}
}

// ������� + �������� ��� � ��� �������������
void INTERNALFUNCTION::getActiveSip(void)
{
	if (!CONSTANTS::DEBUG_MODE)	
	{
		system(CONSTANTS::cActiveSipResponse.c_str());
	}

    ACTIVE_SIP::Parsing sip(CONSTANTS::cActiveSipName.c_str());
	if (sip.isExistList()) 
	{ 
		sip.show(); 
		sip.updateData();
	}	
}

// ��������� ������ �������
std::string INTERNALFUNCTION::getNumberQueue(CONSTANTS::AsteriskQueue queue)
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

// ������� ������ ���-�� ������� � 00:00:00 ������
std::string INTERNALFUNCTION::getTalkTime(std::string talk)
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

// ������� ����� 
std::string INTERNALFUNCTION::getCurrentDateTime()
{	
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);		

	//������ ���-�����-���� 00:00:00
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

// ������� ������ ���
std::string INTERNALFUNCTION::getCurrentStartDay()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	//������ ���-�����-���� 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }	

	return year + "-" + mon + "-" + day + " 00:00:00" ;
}

// ������� ����� - 2 ������ 
std::string INTERNALFUNCTION::getCurrentDateTimeAfterMinutes(int minutes)
{
	auto now = std::chrono::system_clock::now();
	auto minute = std::chrono::minutes(minutes);

	std::time_t now_c = std::chrono::system_clock::to_time_t(now-minute);
	struct std::tm *now_tm = std::localtime(&now_c);

	//������ ���-�����-���� 00:00:00
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

// ������� ����� ����� 20:00 
std::string INTERNALFUNCTION::getCurrentDateTimeAfter20hours()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	//������ ���-�����-���� 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1) { mon = "0" + mon; }

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1) { day = "0" + day; }

	return year + "-" + mon + "-" + day + " 20:00:00";
}

// ����� ������ ����
void INTERNALFUNCTION::showVersionCore(unsigned int iter)
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
	
	// �����������
	if (CONSTANTS::SAFE_LOG) {
		buffer << "\n";
		buffer << "\tLOGGER: ON" << "\t";
		buffer << "DEBUG: " << (to_string(CONSTANTS::LOG_MODE_DEBUG) == "true" ? "ON" : "OFF") << "\t";
		buffer << "ERROR: " << (to_string(CONSTANTS::LOG_MODE_ERROR) == "true" ? "ON" : "OFF") << "\n";
	}

	// �������� ����� �� ����� �����
	buffer << "\t"+getCurrentDateTime() + "\t\titeration: " << iter <<"\n";

	buffer << "==========================================================\n";
	std::cout << buffer.str();
}

// ���������� // ���� ��� ������, ����� ����� � ��������� ����� �������
void INTERNALFUNCTION::getStatistics()
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

// ������������ ���� ��� �� �������� ������������ � ��
void INTERNALFUNCTION::showErrorBD(const std::string str)
{
	std::cerr << str << " -> Error: can't connect to database " << CONSTANTS::cHOST << ":" << CONSTANTS::cBD << "\n";
	if (CONSTANTS::SAFE_LOG) 
	{
		if (CONSTANTS::LOG_MODE_ERROR) 
		{
			LOG::LogToFile log(LOG::eLogType_ERROR);
			log.add(str +" -> Error: can't connect to database " + CONSTANTS::cHOST + ":" + CONSTANTS::cBD);
		}
	}
}

// ����������� ���� ��� ����� ����� �� ������
void INTERNALFUNCTION::showErrorBD(const std::string str, MYSQL *mysql)
{
	std::cerr << str <<" " << mysql_error(mysql) <<"\n";
	if (CONSTANTS::SAFE_LOG) 
	{
		if (CONSTANTS::LOG_MODE_ERROR)	
		{
			LOG::LogToFile log(LOG::eLogType_ERROR);
			log.add(str + " " + mysql_error(mysql));
		}
	}
}

// �������������� ������� ��������� ������ �� int -> REMOTE_COMMANDS::Command 
LOG::Log INTERNALFUNCTION::getRemoteCommand(int command)
{
	return static_cast<LOG::Log>(command);
}

// �������������� ������� ��������� ������ �� LOG::Log -> int
int INTERNALFUNCTION::getRemoteCommand(LOG::Log command)
{
	return static_cast<int>(command);
}

// �������������� ������� ��������� ������ �� REMOTE_COMMANDS::StatusOperators -> int
int INTERNALFUNCTION::getStatusOperators(REMOTE_COMMANDS::ecStatusOperators status)
{
	return static_cast<int>(status);
}


bool INTERNALFUNCTION::isExistNewOnHoldOperators(const SP_OnHold &onHold, const Operators &operators)
{
	int count_operators_active_sip{ 0 };
	for (const auto &list : operators) 
	{
		if (list.isOnHold) 
		{
			++count_operators_active_sip;
		}
	}
	
	return (onHold->size() == count_operators_active_sip);

}

INTERNALFUNCTION::SP_NewOnHoldOperators INTERNALFUNCTION::createNewOnHoldOperators(const SP_OnHold &onHold, const Operators &operators)
{
	SP_NewOnHoldOperators new_lists = std::make_shared<std::map<std::string, std::string>>();

	for (const auto &operators_list : operators) 
	{		
		if (operators_list.isOnHold) 
		{		
			
			const auto &onHoldVec = *onHold; // ���������� ���������
			
			for (size_t i = 0; i != onHoldVec.size(); ++i)
			{
				if (operators_list.sip_number != onHoldVec[i].sip_number)
				{					
					new_lists.get()->insert(std::make_pair(operators_list.sip_number, operators_list.phoneOnHold));
				}
			}			
		}
	}
	
	return new_lists;
}

// �������� ������� �� ��������� ��������� �������
bool INTERNALFUNCTION::remoteCommandChekedExecution(LOG::Log command)
{	
	
	std::ifstream fileRemoteCommand;
	fileRemoteCommand.open(CONSTANTS::cRemoteCommandName);

	// �� ������� ������� ����
	if (!fileRemoteCommand.is_open()) return false;

	// ������� � ��������� ������� 
	std::string line;
	

	while (std::getline(fileRemoteCommand, line)) {		
		
		// ����� ������ ������� ����
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
		else if (command == LOG::Log::Log_del_queue_5000		||
				 command == LOG::Log::Log_del_queue_5050		||
				 command == LOG::Log::Log_del_queue_5000_5050	||
				 command == LOG::Log::Log_home					||
				 command == LOG::Log::Log_exodus				||
				 command == LOG::Log::Log_break					||
				 command == LOG::Log::Log_dinner				||
				 command == LOG::Log::Log_postvyzov				||
				 command == LOG::Log::Log_studies				||
				 command == LOG::Log::Log_IT					||
				 command == LOG::Log::Log_transfer				||
				 command == LOG::Log::Log_reserve				||
				 command == LOG::Log::Log_callback) 
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


bool INTERNALFUNCTION::to_bool(const std::string &str)
{
	return ((str == "true") ? true : false);
}

std::string INTERNALFUNCTION::to_string(bool value)
{
	return (value ? "true" : "false");
}

size_t INTERNALFUNCTION::string_to_size_t(const std::string &str)
{
	std::stringstream stream(str);
	size_t output;
	stream >> output;

	return output;
}

void INTERNALFUNCTION::showHelpInfo()
{
	system("clear");
	//printf("\n\t%s\n", CONSTANTS::core_version);
	std::cout << "\n\t" << CONSTANTS::core_version << "\n";
	std::cout << "\t\t\tList of commands: \n\n";
	std::cout << " ivr \t\t\t - ��� � IVR \n";
	std::cout << " queue \t\t\t - ������� ������� \n";
	std::cout << " active_sip \t\t - ����� �������� sip ���������������� � ������� \n";
	std::cout << " connect_bd \t\t - �������� ���������� � ��\n";
	std::cout << " start \t\t\t - ���� ������ � �� (����� ������� ������� ��� ����������� ������)\n";
	std::cout << " statistics \t\t - ���������� ����������\n";
	std::cout << " housekeeping \t\t - ���������� ������� �� ������� �� ������ (queue, logging, ivr)\n\n";

	std::cout << "\t\t\t\t\t\t\t\t === by Petrov Yuri @2024 === \n\n";
}










