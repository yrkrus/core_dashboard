#ifndef LOG_H
#define LOG_H

#include <string>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
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


class Log 
{
private:
    SP_SQL		    m_sql;    
    std::string     m_name; // файл куда будем писать лог
    std::mutex      m_mutex;    
    std::ofstream   m_file;


    bool            m_ready; // файл готов для записи

    bool GetCommandInfoUser(CommandSendInfoUser &_userInfo, unsigned int _id, std::string &_errorDescription);

    void OpenLogFile();
    void CloseLogFile();

    bool IsReady() const;    

public:    
    Log() = delete;
    Log(const std::string &_name);
    ~Log();   
    
    void ToBase(Command _command, std::string &_errorDescription);      // сохранение в БД
    void ToFile(ELogType _type, const std::string &_message);           // сохранение в лог файл

};

#endif // LOG_H




