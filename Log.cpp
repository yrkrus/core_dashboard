#include "Log.h"
#include "SQLRequest.h"
#include <vector>
#include "Constants.h"
#include "InternalFunction.h"



// создание лога
void LOG::Logging::createLog(Log command, int base_id)
{
	SQL_REQUEST::SQL base;

	// добавляем 
	if (base.isConnectedBD())
	{
		base.addLog(command, base_id);
	}
}



void LOG::LogToFile::add(std::string message)
{
	std::lock_guard<std::mutex> lock(mutex);
	std::string mess = getCurrentDateTime() + "\t" + ELogType_to_string(current_type) + "\t" + message + "\n";

	if (file_log->is_open()) 
	{		
		*file_log << mess;
	}
	
}

LOG::LogToFile::LogToFile(ELogType type)
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
				std::cerr << getCurrentDateTime << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
			case eLogType_INFO:
			{
				std::cerr << getCurrentDateTime << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogINFO << "\n";
				break;
			}
			case eLogType_ERROR:
			{
				std::cerr << getCurrentDateTime << " Ошибка при открытии лог-файла: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
		}
	}	
}

std::string LOG::LogToFile::ELogType_to_string(const ELogType &elogtype)
{
	switch (elogtype)
	{
		case LOG::eLogType_DEBUG: {
			return "DEBUG";
			break;
		}		
		case LOG::eLogType_INFO: {
			return "INFO";
			break;
		}		
		case LOG::eLogType_ERROR: {
			return "ERROR";
			break;
		}		
	}
}

LOG::LogToFile::~LogToFile()
{
	if (file_log->is_open()) 
	{
		file_log->close();		
	}
	delete file_log;
}
