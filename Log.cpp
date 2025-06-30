#include "Log.h"
#include "SQLRequest.h"
#include <vector>
#include "Constants.h"
#include "InternalFunction.h"

using namespace INTERNALFUNCTION;

// создание лога
void LOG_old::Logging::createLog(ecStatus command, int base_id)
{
	SQL_REQUEST::SQL base;

	// добавляем 
	if (base.isConnectedBD())
	{
		base.addLog(command, base_id);
	}
}



void LOG_old::LogToFile::add(std::string message)
{
	std::lock_guard<std::mutex> lock(mutex);	
	std::string mess = getCurrentDateTime() + "\t" + ELogType_to_string(current_type) + "\t" + message + "\n";

	if (file_log->is_open()) 
	{		
		*file_log << mess;
	}	
}


//void LOG::LogToFile::add(const std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> onhold, const std::vector<ACTIVE_SIP_old::Operators_old> *operators)
//{
//	std::lock_guard<std::mutex> lock(mutex);
//
//	std::ostringstream buffer;
//
//	buffer << "========================   " + getCurrentDateTime() + "   ==========================\n";
//
//	unsigned int countHoldBase{ 0 };
//	unsigned int countOperatorsOnHoldAsterisk{ 0 };
//
//	countHoldBase = onhold->size();
//
//	for (const auto &list : *operators)
//	{
//		if (list.isOnHold) ++countOperatorsOnHoldAsterisk;
//	}
//	buffer << "count OnHold Operators Base -> \t"			<< std::to_string(countHoldBase) << "\n";
//	buffer << "count OnHold Asterisk -> \t"					<< std::to_string(countOperatorsOnHoldAsterisk) << "\n";
//
//	if (file_log->is_open())
//	{
//		*file_log << buffer.str();
//	}
//}

//void LOG::LogToFile::add(const std::vector<ACTIVE_SIP_old::OnHold_old> *onhold)
//{
//	std::lock_guard<std::mutex> lock(mutex);
//	
//	std::ostringstream buffer;
//
//	buffer << "========================" + getCurrentDateTime() + "   ==========================\n";
//	
//	unsigned int countHold{ 0 };
//
//	for (const auto &list : *onhold) {
//		if (list.isOnHold) ++countHold;		
//	}
//	buffer << "count OnHold -> " << std::to_string(countHold) << "\n";
//
//	if (file_log->is_open())
//	{
//		*file_log << buffer.str();
//	}
//}

LOG_old::LogToFile::LogToFile(ELogType type)
{
	this->file_log = new std::ofstream;
	current_type = type;

	switch (type)
	{
		case eLogType_DEBUG: {				
			file_log->open(CONSTANTS::cFileLogDEBUG, std::ios_base::app);

			break;
		}
		case eLogType_INFO: {
			file_log->open(CONSTANTS::cFileLogINFO, std::ios_base::app);
			break;
		}
		case eLogType_ERROR: {
			file_log->open(CONSTANTS::cFileLogERROR, std::ios_base::app);
			break;
		}
	}	
	
	if (file_log->is_open()) {
		file_log->seekp(std::ios_base::end); // включаем дозапись лога
	}
	else {
		switch (type)
		{
			case eLogType_DEBUG:
			{
				std::cerr << getCurrentDateTime() << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
			case eLogType_INFO:
			{
				std::cerr << getCurrentDateTime() << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogINFO << "\n";
				break;
			}
			case eLogType_ERROR:
			{
				std::cerr << getCurrentDateTime() << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
		}
	}	
}

std::string LOG_old::LogToFile::ELogType_to_string(const ELogType &elogtype)
{
	switch (elogtype)
	{
		case LOG_old::eLogType_DEBUG: {
			return "DEBUG";
			break;
		}		
		case LOG_old::eLogType_INFO: {
			return "INFO";
			break;
		}		
		case LOG_old::eLogType_ERROR: {
			return "ERROR";
			break;
		}		
	}
}

LOG_old::LogToFile::~LogToFile()
{
	if (file_log->is_open()) 
	{
		file_log->close();		
	}
	delete file_log;
}
