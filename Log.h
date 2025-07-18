#ifndef LOG_H
#define LOG_H

#include <string>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
#include "ActiveSip.h"
#include "ISQLConnect.h"


#define LOG_NAME_DEFAULT "unnamed_log.log"

struct Command;
struct CommandSendInfoUser;

enum class ELogType
{
    Debug,
    Info,
    Error,
};

using SP_FileLog = std::shared_ptr<std::ofstream>;

class Log 
{
private:
    SP_SQL		m_sql;    
    std::string m_name; // файл куда будем писать лог
    std::mutex  m_mutex;    
    SP_FileLog  m_file;    


    bool        m_ready; // файл готов для записи

    bool GetCommandInfoUser(CommandSendInfoUser &_userInfo, unsigned int _id, std::string &_errorDescription);

    void OpenLogFile();
    void CloseLogFile();

    bool IsReady() const;    

public:    
    Log();
    Log(const std::string &_name);
    ~Log();   
    
    void ToBase(Command _command, std::string &_errorDescription);      // сохранение в БД
    bool ToFile(ELogType _type, const std::string &_request);                   // сохранение в лог файл

};
using SP_Log = std::shared_ptr<Log>;

namespace LOG_old {	
	
	
	class Logging_old
	{
    public:
        Logging_old()	= default;
		~Logging_old()	= default;
    
       // void createLog(remote::ECommand command, int base_id);       // создание лога
	};

    
    enum ELogType_old
    {
        eLogType_DEBUG  = 1,
        eLogType_INFO   = 2,
        eLogType_ERROR  = 3,
    };


    class LogToFile_old
    {
    public:
        LogToFile_old(ELogType_old type);
        ~LogToFile_old();
            
        void add(std::string message);  // сохранение лога в файл 
        /*void add(const std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> onhold, const std::vector<ACTIVE_SIP_old::Operators_old> *operators);
        void add(const std::vector<ACTIVE_SIP_old::OnHold_old> *onhold);*/

    private:        
        mutable std::mutex mutex;
        std::ofstream *file_log = nullptr;
        ELogType_old current_type;

        std::string ELogType_to_string(const ELogType_old &elogtype);    
    };   
}

#endif // LOG_H




