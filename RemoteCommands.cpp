#include "RemoteCommands.h"
#include "Log.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "Queue.h"
#include "utils.h"

using namespace utils;


Status::Status()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_dispether("status",CONSTANTS::TIMEOUT::OPERATOR_STATUS)
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::REMOTE_COMMANDS))
{
}

Status::~Status()
{
}

void Status::Start()
{
	m_dispether.Start([this]()
		{
			return Execute();
		});
}

void Status::Stop()
{
	m_dispether.Stop();
	printf("Status stopped!\n");
}

bool Status::IsExistCommand()
{
	return !m_commandList.empty() ? true : false;
}

// получение новых команд из БД
bool Status::GetCommand(std::string &_errorDesciption)
{
	_errorDesciption.clear();
	if (!m_commandList.empty()) 
	{
		m_commandList.clear();
	}
	
	const std::string query = "select id,sip,command,user_id,delay,pause from remote_commands where error = '0' ";

	if (!m_sql->Request(query, _errorDesciption))
	{
		m_sql->Disconnect();
		return false;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Command command;
		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0: command.id = std::stoi(row[i]);								break;
				case 1: command.sip = row[i];										break;
				case 2: command.command = GetRemoteCommand(std::stoi(row[i]));		break;
				case 3: command.userId = std::stoi(row[i]);							break;
				case 4: command.delay  = (std::stoi(row[i]) == 1 ? true : false);	break;
				case 5: command.pause  = (std::stoi(row[i]) == 1 ? true : false);	break;
			}				
		}

		// добавим найденные команды
		m_commandList.push_back(command);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

// выполнение команды
bool Status::ExecuteCommand(const Command &_command, std::string &_errorDesciption)
{	
	// найдем команду (add\del\pause)
	ecCommandType commandType;

	commandType = (_command.delay && !_command.pause) ? ecCommandType::Pause
													  : GetCommandType(_command);

	// такого быть не должно но все же
	if (commandType == ecCommandType::Unknown) 
	{
		_errorDesciption = StringFormat("remote command %s not support",EnumToString<ecCommand>(_command.command).c_str());
		return false;
	}
	
	std::string rawCommandStr;	// команда по умолчанию на отправку

	switch (commandType)
	{
		case(ecCommandType::Add): rawCommandStr = COMMAND_ADD_QUEUE;	break;
		case(ecCommandType::Del): rawCommandStr = COMMAND_DEL_QUEUE;	break;	
		case(ecCommandType::Pause): 
		{
			// выполнение команды на добавление в pause на все очереди
			return ExecuteCommandPause(_command, _errorDesciption);		
		}
		default:		
			return false;
	}	
	
	// найдем очередь
	ecQueueNumber queue = GetQueueNumber(_command.command);

	// такого не должно быть но все же
	if (queue == ecQueueNumber::eUnknown)
	{
		// TODO в лог не забыть писать
		return false;
	}

	std::string request;

	// для двойной очереди или для входа в статус не связанный с очередью 
	if (queue == ecQueueNumber::e5000_e5050)
	{
		for (size_t i = 1; i < 3; ++i)
		{
			ecQueueNumber queue = static_cast<ecQueueNumber>(i);

			request = CreateCommand(_command, queue, rawCommandStr);

			if (!SendCommand(commandType, request, _errorDesciption))
			{
				return false;
			}
		}
	}
	else
	{
		request = CreateCommand(_command, queue, rawCommandStr);

		if (!SendCommand(commandType, request, _errorDesciption))
		{
			return false;
		}
	}	

	// добавим в лог запрос
	m_log->ToBase(_command);

	// обновим текущий статус оператора
	if (!UpdateNewStatus(_command, _errorDesciption))
	{
		_errorDesciption = StringFormat("%s\t%s",METHOD_NAME, _errorDesciption.c_str());
		m_log->ToFile(ecLogType::eError, _errorDesciption);

		return false;
	}

	return true;
}

bool Status::ExecuteCommandPause(const Command &_command, std::string &_errorDesciption)
{
	std::string rawRequest = COMMAND_PAUSE_QUEUE;	
	std::string request = CreateCommand(_command, rawRequest); 	 
	
	if (!SendCommand(ecCommandType::Pause, request, _errorDesciption))
	{
		return false;
	}

	return true;
}

std::string Status::CreateCommand(const Command &_command, const ecQueueNumber _queue, const std::string &_rawCommand)
{
	std::string request = _rawCommand;

	// сформируем строку (номер очереди)
	ReplaceResponseStatus(request, "%queue", EnumToString<ecQueueNumber>(_queue));

	// сформируем строку (sip очереди)
	ReplaceResponseStatus(request, "%sip", _command.sip);	

	return request;
}

std::string Status::CreateCommand(const Command &_command, const std::string &_rawCommand)
{
    std::string request = _rawCommand;

	// сформируем строку (sip очереди)
	ReplaceResponseStatus(request, "%sip", _command.sip);	

	return request;
}

void Status::DeleteCommand(const Command &_command)
{
	const std::string query = "delete from remote_commands where id = '" + std::to_string(_command.id) 
																		 + "' and error = '0'"
																		 + " and sip = '" + _command.sip + "'";

	std::string error;
	if (!m_sql->Request(query, error)) 
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();	
}

// не удачное выполнение команды
void Status::ExecuteCommandFail(const Command &_command, const std::string &_errorStr)
{
	std::string error;	
	const std::string query = "update remote_commands set error = '1', error_str = '"+_errorStr
											+"'  where id = '" + std::to_string(_command.id)
											+"' and sip = '" + _command.sip + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);

		m_sql->Disconnect();				
	}

	m_sql->Disconnect();	
}

bool Status::SendCommand(ecCommandType commandType, std::string &_request, std::string &_errorDesciption)
{
	m_register.DeleteRawAll(); // очистим все текущие данные 

	if (!m_register.CreateData(_request, _errorDesciption))
	{
		return false;
	}	

	return CheckSendingCommand(commandType, _errorDesciption);
}

// проверка успешно ли выполнена команда
bool Status::CheckSendingCommand(ecCommandType _commandType, std::string &_errorDesciption)
{
	bool status = false;

	// проверим ответ
	if (!m_register.IsExistRaw())
	{
		_errorDesciption = "not respond asterisk command";
		return false;
	}

	std::string rawLines = m_register.GetRawFirst();
	if (rawLines.empty())
	{
		// TODO тут подумать, что делать!
		return true;
	}

	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))
	{
		switch (_commandType)
		{
		case ecCommandType::Unknown:
			_errorDesciption = StringFormat("command %s not found", EnumToString<ecCommandType>(_commandType).c_str());
			break;
		case ecCommandType::Del:
			if ((line.find("Removed") != std::string::npos) || (line.find("Not there") != std::string::npos)) 
			{
				status = true;
			}
			break;
		case ecCommandType::Add:
			if ((line.find("Added") != std::string::npos) || (line.find("Already there") != std::string::npos)) 
			{
				status = true;
			}
			break;
		case ecCommandType::Pause: 
			if (line.find("paused") != std::string::npos) 
			{
				status = true;
			}
			break;
		}
	}
	// очищаем
	m_register.DeleteRawAll();

	return status;
}

bool Status::UpdateNewStatus(const Command &_command, std::string &_errorDesciption)
{	
	// новый статус оператора
	EStatus status;

	switch (_command.command)
	{	
	case ecCommand::AddQueue5000:
	case ecCommand::AddQueue5050:	
	case ecCommand::AddQueue5000_5050:		
		status = EStatus::Available;
		break;

	case ecCommand::DelQueue5000:		
	case ecCommand::DelQueue5050:		
	case ecCommand::DelQueue5000_5050:
		status = EStatus::Unknown;
		break;		
	
	case ecCommand::Home:		status = EStatus::Home;		break;
	case ecCommand::Exodus:		status = EStatus::Exodus;	break;	
	case ecCommand::Break:		status = EStatus::Break;	break;
	case ecCommand::Dinner:		status = EStatus::Dinner;	break;
	case ecCommand::Postvyzov:	status = EStatus::Postvyzov; break;
	case ecCommand::Studies:	status = EStatus::Studies;	break;
	case ecCommand::IT:			status = EStatus::It;		break;
	case ecCommand::Transfer:	status = EStatus::Transfer; break;
	case ecCommand::Reserve:	status = EStatus::Reserve;	break;
	case ecCommand::Callback:	status = EStatus::Callback; break;
	
	default:
		status = EStatus::Unknown;
		break;
	}		

	const std::string query = "update operators set status = '" + std::to_string(static_cast<int>(status)) 
																+ "' where user_id = '" + std::to_string(_command.userId) + "'";

	
	if (!m_sql->Request(query, _errorDesciption))
	{
		m_sql->Disconnect();		
		_errorDesciption = StringFormat("%s",query.c_str());
		return false;
	}

	m_sql->Disconnect();

	return true;
}

bool Status::IsTalkOperator(const std::string &_sip, std::string &_errorDesciption)
{
	const std::string query = "select count(phone) from queue where date_time > '" + GetCurrentStartDay() +
																	"' and sip = '" + _sip + "' and answered = '1' and hash is null limit 1";
	if (!m_sql->Request(query, _errorDesciption))
	{
		// TODO в лог
		printf("%s", _errorDesciption.c_str());
		m_sql->Disconnect();
		// ошибка считаем что нет запись
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool exist;
	std::stoi(row[0]) == 0 ? exist = false : exist = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return exist;
}

void Status::CreatePauseQueue(const Command &_command, std::string &_errorDesciption)
{
	if (!ExecuteCommand(_command, _errorDesciption))
	{
		m_log->ToFile(ecLogType::eError, _errorDesciption);
		return;		
	}
	
	// обновим параметр pause
	const std::string query = "update remote_commands set pause = '1' where user_id = '" + std::to_string(_command.userId) + "'";
	
	if (!m_sql->Request(query, _errorDesciption))
	{
		m_sql->Disconnect();		
		m_log->ToFile(ecLogType::eError, _errorDesciption);		
	}

	m_sql->Disconnect();
}

// поиск какая команда пришла
ecCommandType Status::GetCommandType(const Command &_command)
{	
	// TODO может в будущем потом на std::map и static переделать
	
	switch (_command.command)
	{
		case ecCommand::Enter:
		case ecCommand::Exit:
		case ecCommand::AuthError:
		case ecCommand::ExitForce: 
						return ecCommandType::Unknown;				
		
		case ecCommand::AddQueue5000:
		case ecCommand::AddQueue5050:
		case ecCommand::AddQueue5000_5050: 
						return ecCommandType::Add;
		
		case ecCommand::DelQueue5000: 
		case ecCommand::DelQueue5050:
		case ecCommand::DelQueue5000_5050:	
		case ecCommand::Available:
		case ecCommand::Home: 
		case ecCommand::Exodus: 
		case ecCommand::Break:
		case ecCommand::Dinner: 
		case ecCommand::Postvyzov: 
		case ecCommand::Studies: 
		case ecCommand::IT: 
		case ecCommand::Transfer:
		case ecCommand::Reserve:
		case ecCommand::Callback: 
						return ecCommandType::Del;
	
	default:
		return ecCommandType::Unknown;
	}	
}

// поиск номера очереди
ecQueueNumber Status::GetQueueNumber(const ecCommand &_command)
{
	switch (_command)
	{	
		case ecCommand::AddQueue5000:
		case ecCommand::DelQueue5000:
			return ecQueueNumber::e5000;

		case ecCommand::AddQueue5050:
		case ecCommand::DelQueue5050:
			return ecQueueNumber::e5050;

		case ecCommand::AddQueue5000_5050:
		case ecCommand::DelQueue5000_5050:		
		case ecCommand::Home:
		case ecCommand::Exodus:
		case ecCommand::Break:
		case ecCommand::Dinner:
		case ecCommand::Postvyzov:
		case ecCommand::Studies:
		case ecCommand::IT:
		case ecCommand::Transfer:
		case ecCommand::Reserve:
		case ecCommand::Callback:
			return ecQueueNumber::e5000_e5050;
	
	default:
			return ecQueueNumber::eUnknown;
	}
}

//EStatus Status::GetStatus(const ECommand &_command)
//{
//	return EStatus();
//}

// выполнение команд
bool Status::Execute()
{
	std::string error;
	if (!GetCommand(error)) 
	{		
		m_log->ToFile(ecLogType::eError, error);		
		return false;
	}
	
	// выполнение команд
	if (IsExistCommand()) 
	{
		for (const auto &command : m_commandList)
		{
			std::string error;

			if (command.delay)	// отложенное выполнение команды
			{
				// проверим разговаривает ли еще оператор или нет
				if (IsTalkOperator(command.sip, error))  
				{
					// поставим на pause, что бы все будущие звонки не приходили
					if (!command.pause)
					{
						// стаим на паузу новые звонки
						CreatePauseQueue(command, error);
					}					
					
					// пропускаем команду, т.к. оператор еще разговаривает
					continue;
				}				
			}
						
			if (!ExecuteCommand(command, error))
			{
				m_log->ToFile(ecLogType::eError, error);

				// не удачно выполнили команду, нужно удалить ее сразу т.к. у пользвака не будет надписи об ошибке
				if (command.delay) 
				{
					DeleteCommand(command);
					continue;
				}

				// инфо в БД что не успешно выполнили команду, дальше в gui это отображается у пользователя
				ExecuteCommandFail(command, error);
				continue;
			}
			else
			{
				// успешно отработанная команда удаляем ее
				DeleteCommand(command);
			}							
		}
	}

	return true;
}
