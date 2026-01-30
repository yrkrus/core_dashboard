#include <iostream>
#include <fstream>
#include "CallbackCall.h"
#include "../system/Constants.h"
#include "../utils/InternalFunction.h"

using utils::StringFormat;
using namespace utils;

#define FILE_CALL_NAME ".call"

// /var/spool/asterisk/outgoung/*.call

	//*.call
	//Channel: SIP/druOUT_220220/89275052333
	//CallerID: "Dasboard Missed Call" <89275052333>
	//WaitTime: 20
	//Extension: 64197
	//Priority: 1     


CallbackCall::Command::Command()  
        : id(0)
        , user_id(0)             
        , phone{}        
        , waiting_time{} 
        , date_time{}
        , sip(-1)
        , create_call(false)
        , talk_time{} 
        , answered(false)         
        , hash(0)
        , call_id{} 
{
}     
         

CallbackCall::CallbackCall()
    : m_sql(std::make_shared<ISQLConnect>(false))
    , m_dispether("callbackcall", CONSTANTS::TIMEOUT::CALLBACKCALL)
    , m_log(std::make_shared<Log>(CONSTANTS::LOG::CALLBACKCALL))
{
}

CallbackCall::~CallbackCall()
{
}

void CallbackCall::Start()
{
    m_dispether.Start([this]()
                      { return Execute(); });
}

void CallbackCall::Stop()
{
    m_dispether.Stop();
	printf("CallbackCall stopped!\n");
}

bool CallbackCall::Execute()
{
    std::string errorDescription;
    if (!GetCommand(errorDescription))
    {
        m_log->ToFile(ecLogType::eError, errorDescription);
        return false;
    }

    if (!IsExistCommand()) 
    {
        // новых данных нет // TODO еще проверить 
        return true;
    }

    // есть новые команды, нужно создать *.call файл
    if (!CreateCallFile(errorDescription))
    {
        m_log->ToFile(ecLogType::eError, errorDescription);
        return false;   
    }


    return true;
}

// получение новых команд из БД
bool CallbackCall::GetCommand(std::string &_errorDescription)
{
	_errorDescription.clear();
	if (!m_commandList.empty()) 
	{
		m_commandList.clear();
	}
	
//	const std::string query = "select id,user_id,phone,waiting_time,date_time,sip,create_call,talk_time,answered,hash,call_id from queue_outgoing where create_call = '0' and hash is null and call_id is NULL";
    const std::string query = "select id,user_id,phone,waiting_time,date_time,sip,create_call,talk_time,answered,hash,call_id from queue_outgoing where hash is null";


	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		return false;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		_errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, _errorDescription);
		m_sql->Disconnect();
		return false;
	}

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Command command;
		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0: command.id = std::stoi(row[i]);							            break;
                case 1: command.user_id = std::stoi(row[i]);					            break;
                case 2: command.phone = row[i];						                        break;
                case 3: if (row[i]) command.waiting_time = row[i];                          break;
                case 4: command.date_time = row[i];                                         break;
                case 5: command.sip = std::stoi(row[i]);	                                break;
                case 6: command.create_call = (std::stoi(row[i]) == 1 ? true : false);     break;
                case 7: if (row[i]) command.talk_time = row[i];                             break;
                case 8: command.answered = (std::stoi(row[i]) == 1 ? true : false);         break;
                case 9: if (row[i]) command.hash = string_to_size_t(row[i]);                break;
                case 10: if (row[i]) command.call_id = row[i];                              break;         
    		}				
		}

		// добавим найденные команды
		m_commandList.push_back(command);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

bool CallbackCall::IsExistCommand() const
{
    return !m_commandList.empty();
}

bool CallbackCall::CreateCallFile(std::string &_errorDescription)
{
    for (const auto &command : m_commandList) 
    {
        if (!CreateFile(command, _errorDescription)) 
        {            
            return false;
        }

    }
    
    
    
    return true;
}

bool CallbackCall::CreateFile(const Command &_command, std::string &_errorDescription)
{
    std::string fileName = std::to_string(_command.id) + "_" + std::to_string(_command.user_id) + "_" + _command.phone + FILE_CALL_NAME;
    std::ofstream callFile_temp(fileName.c_str()); 
    if (!callFile_temp.is_open()) 
    {
        _errorDescription = StringFormat("failed to create file %s", fileName.c_str());
        return false;
    }
    
    return true;

}
