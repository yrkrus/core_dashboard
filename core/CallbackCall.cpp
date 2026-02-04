#include "CallbackCall.h"
#include <iostream>
#include <fstream>
#include "CallbackCall.h"
#include "../system/Constants.h"
#include "../utils/custom_cast.h"
#include "../utils/InternalFunction.h"

using utils::StringFormat;
using namespace utils;
using namespace custom_cast;

#define FILE_CALL_NAME ".call"
#define CALL_WHAIT_TIME 20                                  // время в сек при котором будем звонить на номер
#define CALL_FILE_DESTINATION "/var/spool/asterisk/outgoing/" 

static std::string SIP_COMMAND		        = "callback_";	// поиск по этой строке
static std::string SESSION_SIP_RESPONSE 	= "asterisk -rx \"core show channels concise\"" " | grep -E \"" + SIP_COMMAND + "\"";

// /var/spool/asterisk/outgoing/*.call

	//*.call
	//Channel: SIP/druOUT_220220/89275052333
	//CallerID: "Dasboard Missed Call" <CallBack_89275052333>
	//WaitTime: 20
	//Extension: 64197
	//Priority: 1  

CallbackCall::Command::Command()  
        : id(0)
        , user_id(0)             
        , phone{}      
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
        // новых данных нет // TODO еще подумать что с этим потом делать 
        return true;
    }

    // есть новые команды, нужно создать *.call файл
    CreateCallFile();

  
    // работаем с активными звонками 
    ActiveCallbackCall();
   

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
    const std::string query = "select id,user_id,phone,date_time,sip,create_call,talk_time,answered,hash,call_id from queue_outgoing where hash is null and error = '0'";


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
                case 3: command.date_time = row[i];                                         break;
                case 4: command.sip = std::stoi(row[i]);	                                break;
                case 5: command.create_call = (std::stoi(row[i]) == 1 ? true : false);      break;
                case 6: command.talk_time = row[i];                                         break;
                case 7: command.answered = (std::stoi(row[i]) == 1 ? true : false);         break;
                case 8: if (row[i]) command.hash = string_to_size_t(row[i]);                break;
                case 9: if (row[i]) command.call_id = row[i];                               break;         
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

void CallbackCall::CreateCallFile()
{
    std::string errorDescription;

    for (auto &command : m_commandList)
    {
        if (!command.create_call)
        {
            if (!CreateFile(command, errorDescription))
            {
                m_log->ToFile(ecLogType::eError, errorDescription);
                continue;
            }

            // отметим в БД что файл успешно создан
            CreateCallFileSucess(command);
        }
    }  
}

bool CallbackCall::CreateFile(const Command &_command, std::string &_errorDescription)
{
    std::string fileName = std::to_string(_command.id) + "_" + std::to_string(_command.user_id) + "_" + _command.phone + FILE_CALL_NAME;
    std::string fullpath(CALL_FILE_DESTINATION + fileName);

    std::ofstream callFile_temp(fullpath.c_str()); 
    if (!callFile_temp.is_open()) 
    {
        _errorDescription = StringFormat("failed to create file %s", fullpath.c_str());
        return false;
    }      
   
    std::string call =  std::string{R"(Channel: SIP/druOUT_220220/)"} + _command.phone + "\n"
                                  + R"(CallerID: "Callback Call" <callback_)" + _command.phone + ">\n"
                                  + R"(WaitTime: )" + std::to_string(CALL_WHAIT_TIME) + "\n"
                                  + R"(Extension: )" + std::to_string(_command.sip) + "\n"
                                  + R"(Priority: 1)";    

    callFile_temp << call;
    callFile_temp.close();                  

    return true;
}

void CallbackCall::CreateCallFileSucess(Command &_command)
{
    const std::string query = std::string("update queue_outgoing set create_call = '1")									 
										+ "' where id = '" + std::to_string(_command.id) 
										+ "' and user_id = '" + std::to_string(_command.user_id) 
                                        + "' and phone = '"+ _command.phone + "'";                               

	std::string errorDescription;
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();

    // установим новый параметр
    _command.create_call = true;
}

int CallbackCall::GetCommandID(const std::string &_phone)
{
    for (const auto &it : m_commandList)
    {
        if (it.phone == _phone)
        {
            return it.id;
        }
    }

    return 0;
}

void CallbackCall::UpdateActiveCallHash(const CommandList &_commandList)
{
    for (const auto &command : _commandList) 
    {
        std::string errorDescription;
        if (!UpdateCallHash(command, errorDescription)) 
        {           
		    m_log->ToFile(ecLogType::eError, errorDescription);  
        }
    }
}

void CallbackCall::ActiveCallbackCall()
{
    CommandList commandList = GetActiveCallbackCall();

    if (commandList.empty()) 
    {
        // TODO тут подумать что то еще точно надо 
        return;
    }

    // обновляем данные по астеру
    std::string errorDescription;
	if (!CreateRawData(errorDescription))
	{
		// если raw данных нет значит нет активных звонков надо hash обновить
        UpdateActiveCallHash(commandList);
		return;
	}

    // разбираем данные
    ParsingCallbackCall();

}

CallbackCall::CommandList CallbackCall::GetActiveCallbackCall()
{
    CommandList commandList;

    for (auto &call : m_commandList) 
    {
        if ((call.create_call) && (call.hash == 0)) 
        {
            commandList.push_back(call);
        }
    }

    return commandList;
}

bool CallbackCall::CreateRawData(std::string &_errorDescription)
{
    bool test =  m_rawData.CreateData(SESSION_SIP_RESPONSE,_errorDescription);
//	m_log->ToFile(ecLogType::eInfo, StringFormat("m_raw = %s", m_rawData.GetRawLast().c_str()));
	return test;

//	return m_rawData.CreateData(SESSION_SIP_RESPONSE,_errorDescription);
}

void CallbackCall::ParsingCallbackCall()
{
    if (!m_rawData.IsExistRaw()) 
    {
        return;
    }

    // обнулим данные
    m_call.clear();
    
    std::string rawLines = m_rawData.GetRawFirst();    

    std::istringstream ss(rawLines);
    std::string line;

    while (std::getline(ss, line))
    {     
		StructActiveCallbackCall caller;

        if (CreateCallers(line, caller))
        {
            m_call.push_back(caller);
        }
        else
        {
            // удаляем из сырых данных
            m_rawData.DeleteRawLast();
            break;
        }
    }

    // что то есть нужно теперь в БД запихнуть
    if (!m_call.empty())
    {
        UpdateActiveCallbackCall(m_call);

        // удаляем из сырых данных
        m_rawData.DeleteRawLast();
    }
}

bool CallbackCall::CreateCallers(const std::string &_lines, StructActiveCallbackCall &_caller)
{
    std::vector<std::string> lines;
	std::string errorDescription;
	if (!utils::ParsingAsteriskRawDataRequest(lines, _lines, errorDescription))
	{
		return false;
	}	

	//utils::ShowVectorLines(lines);

    try
    {
        _caller.phone = lines[7].substr(SIP_COMMAND.length(), lines[7].size());
        _caller.talkTime = static_cast<uint16_t>(std::stoi(lines[11]));
        _caller.status = StringToEnum<ecAsteriskState>(lines[4]);
        _caller.app = StringToEnum<ecAsteriskApp>(lines[5]);
        _caller.call_id = lines[13];
    }    
    catch(const std::exception& e)
    {
        errorDescription = StringFormat("%s \t %s", METHOD_NAME, e.what());
		m_log->ToFile(ecLogType::eError, errorDescription);       
        return false;
    }
    
    if (!_caller.check()) 
    {
        errorDescription = StringFormat("%s \t %s", METHOD_NAME, _lines.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);       
        return false;    
    }

	return true;   
}

void CallbackCall::UpdateActiveCallbackCall(const ActiveCallbackCallList &_calls)
{
    if (_calls.empty()) 
    {
        // TODO сделать

        return;
    }

    for (const auto &call : _calls) 
    {
        std::string errorDescription;
        int id = GetCommandID(call.phone);
        if (id == 0) 
        {
            continue;
        }

        if (!UpdateCallBase(id, call, errorDescription)) 
        {
           m_log->ToFile(ecLogType::eError, errorDescription);       
        }
    }
}

bool CallbackCall::UpdateCallBase(int _id, const StructActiveCallbackCall &_call, std::string &_errorDesciption)
{
    const std::string query = "update queue_outgoing set talk_time = '" + std::to_string(_call.talkTime) 
										+ "', answered = '" + std::to_string((_call.status == ecAsteriskState::Up) ? 1 : 0) 
										+ "', call_id = '" + (_call.call_id) 
                                        + "' where phone = '" + _call.phone 
										+ "' and id = '" + std::to_string(_id) + "'";  
                                    
                                    

	if (!m_sql->Request(query, _errorDesciption))
	{	
		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect(); 
    return true;  
}

bool CallbackCall::UpdateCallHash(const Command &_command, std::string &_errorDesciption)
{
    size_t hash = std::hash<std::string>()(_command.phone + "_" + _command.date_time);

    const std::string query = "update queue_outgoing set hash = '" + std::to_string(hash)  
									    + "' where phone = '" + _command.phone 
										+ "' and id = '" + std::to_string(_command.id) + "'";  

	if (!m_sql->Request(query, _errorDesciption))
	{	
		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect(); 
    return true;  
}
