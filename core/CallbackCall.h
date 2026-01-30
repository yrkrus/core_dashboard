/**
 * @file CallbackCall.h
*/

#pragma once
#include <string>
#include <vector>
#include "../interfaces/ISQLConnect.h"
#include "../interfaces/IPotokDispether.h"
#include "../interfaces/ISQLConnect.h"
#include "../system/Log.h"


struct CallCommand
{
	int		    id;			    // id команды (для удобного поиска в запросе)
	int         user_id;        // id пользователя
    std::string phone;          // номер тлф на который будем звонить
    std::string waiting_time;   // ожидание при звонкке (по факту сколько блоы гудков до момента взятия трубки)
	std::string date_time;      
    int         sip;
    bool        created_call;
    std::string talk_time;      // время разговора
    bool        answered;       // отвечен на звонок
    size_t      hash;
    std::string call_id;        // id звонка по aster   
};

using CommandList = std::vector<CallCommand>;

class CallbackCall 
{
    public:
    CallbackCall();
    ~CallbackCall();

	void Start();
	void Stop();    
    
    private:
    CommandList         m_command;    
    SP_SQL				m_sql;
	IPotokDispether		m_dispether;
	SP_Log				m_log;

    bool Execute();	

};


