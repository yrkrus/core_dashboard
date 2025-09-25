#ifndef LOG_H
#define LOG_H

#include <string>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
#include "../interfaces/ISQLConnect.h"


#define LOG_NAME_DEFAULT "unnamed_log.log"
#define LOG_FOLDER "./log"

struct Command;
struct CommandSendInfoUser;

enum class ecLogType
{
    eDebug,
    eInfo,
    eError,
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
    
    void CreteFolderLog(const std::string &_name);  // проверка что есть папка с логами

public:    
    Log() = delete;
    Log(const std::string &_name);
    ~Log();   
    
    void ToBase(Command _command);                                // сохранение в БД
    void ToFile(ecLogType _type, const std::string &_message);     // сохранение в лог файл
    void ToPrint(const std::string &_message);                    // только отображение в cout<<

};

#endif // LOG_H




