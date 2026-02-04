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
#include "../interfaces/IFile.h"
#include "../interfaces/IAsteriskApplication.h"

class CallbackCall;
using SP_CallbackCall = std::shared_ptr<CallbackCall>;



// структура звонка
struct StructActiveCallbackCall
{
	std::string phone;	   // текущий номер телфеона с которым ведется беседа
	int talkTime = 0;		   // время развговора
	ecAsteriskState status = ecAsteriskState::Unknown;
	ecAsteriskApp app = ecAsteriskApp::Unknown;
	std::string call_id; // id звонка

	inline bool check() const noexcept
	{
		return ((status != ecAsteriskState::Unknown) &&
		 		(app != ecAsteriskApp::Unknown) &&              
				(!phone.empty()) &&			
				(!call_id.empty()));
	}
};
using ActiveCallbackCallList = std::vector<StructActiveCallbackCall>;


class CallbackCall 
{
    struct Command
    {
            int id;                   // id команды (для удобного поиска в запросе)
            int user_id;              // id пользователя
            std::string phone;        // номер тлф на который будем звонить           
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
    CommandList             m_commandList;    
    SP_SQL				    m_sql;
	IPotokDispether		    m_dispether;
	SP_Log				    m_log;
    IFile				    m_rawData;				// список с RAW данными по активному звонку 	
    ActiveCallbackCallList  m_call;                 // список с данными по активному звонку


    bool Execute();	
    bool GetCommand(std::string &_errorDesciption);                             // получение новых команд
    bool IsExistCommand() const;                                                // есть ли новые пустые данные 
    void CreateCallFile();                                                      // создаем *.call файл и перемещаем его в 
    bool CreateFile(const Command &_command, std::string &_errorDesciption);    // создание файла *.call
    void CreateCallFileSucess(Command &_command);                               // успешно создали *.call файл   
    int GetCommandID(const std::string &_phone);    // id команды 
    void UpdateActiveCallHash(const CommandList &_commandList); // обновление hash 

    // --- методы для работы со звонком ---
    void ActiveCallbackCall();                                                  // метод работы с активными звонками callback
    CallbackCall::CommandList GetActiveCallbackCall();                          // наблюдаемые активные звонки //TODO  может стоит const сделать?
    bool CreateRawData(std::string &_errorDescription);			                // получим raw данные
    void ParsingCallbackCall();                                                 // разбор raw данных
    bool CreateCallers(const std::string&, StructActiveCallbackCall&);
    void UpdateActiveCallbackCall(const ActiveCallbackCallList&);               // обновление данных по активному звонку
    
    
    // -- работа с БД --- 
    bool UpdateCallBase(int _id, const StructActiveCallbackCall&, std::string &_errorDesciption); // обновление данных в БД 
    bool UpdateCallHash(const Command &_command, std::string &_errorDesciption);   // обновление данных в БД (hash) 
};


