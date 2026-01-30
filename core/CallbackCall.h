/**
 * @file CallbackCall.h
*/

#pragma once
#include <string>
#include <vector>
#include <ostream>
#include "../interfaces/ISQLConnect.h"
#include "../interfaces/IPotokDispether.h"
#include "../interfaces/ISQLConnect.h"
#include "../system/Log.h"

class CallbackCall;
using SP_CallbackCall = std::shared_ptr<CallbackCall>;



class CallbackCall 
{
    struct Command
    {
            int id;                   // id команды (для удобного поиска в запросе)
            int user_id;              // id пользователя
            std::string phone;        // номер тлф на который будем звонить
            std::string waiting_time; // ожидание при звонкке (по факту сколько блоы гудков до момента взятия трубки)
            std::string date_time;
            int sip;
            bool create_call;
            std::string talk_time; // время разговора
            bool answered;         // отвечен на звонок
            size_t hash;
            std::string call_id; // id звонка по aster
        
            Command();    
    };
   

    using CommandList = std::vector<Command>;

    public:
    CallbackCall();
    ~CallbackCall();

	void Start();
	void Stop();    
    
    private:
    CommandList         m_commandList;    
    SP_SQL				m_sql;
	IPotokDispether		m_dispether;
	SP_Log				m_log;

    bool Execute();	
    bool GetCommand(std::string &_errorDesciption);  // получение новых команд
    bool IsExistCommand() const; // есть ли новые пустые данные 
    bool CreateCallFile(std::string &_errorDesciption); // создаем *.call файл и перемещаем его в 
    bool CreateFile(const Command &_command, std::string &_errorDesciption);   // создание файла *.call

    

};


