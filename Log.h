//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//			  логирование действий   				//
//													//	
//////////////////////////////////////////////////////

#pragma once
#include <string>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
#include "ActiveSip.h"
#include "RemoteCommands.h"


class Log 
{
private:
    Log();
    ~Log();
public:
};


namespace LOG_old {	
	
	
	class Logging
	{
    public:
        Logging()	= default;
		~Logging()	= default;
    
        void createLog(remote::ecCommand command, int base_id);       // создание лога
	};

    
    enum ELogType
    {
        eLogType_DEBUG  = 1,
        eLogType_INFO   = 2,
        eLogType_ERROR  = 3,
    };


    class LogToFile
    {
    public:
        LogToFile(ELogType type);
        ~LogToFile();
            
        void add(std::string message);  // сохранение лога в файл 
        /*void add(const std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> onhold, const std::vector<ACTIVE_SIP_old::Operators_old> *operators);
        void add(const std::vector<ACTIVE_SIP_old::OnHold_old> *onhold);*/

    private:        
        mutable std::mutex mutex;
        std::ofstream *file_log = nullptr;
        ELogType current_type;

        std::string ELogType_to_string(const ELogType &elogtype);    
    };   
}




