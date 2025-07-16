#include "RemoteCommands.h"
#include "SQLRequest.h"
#include "InternalFunction.h"
#include "utils.h"

using namespace utils;
using namespace remote;

// construct
//REMOTE_COMMANDS_old::Remote::Remote()
//{	
//	// формируем лист команд при 
//	createListCommands();
//
//}

//bool REMOTE_COMMANDS_old::Remote::chekNewCommand()
//{
//	SQL_REQUEST::SQL base;
//	if (base.isConnectedBD())
//	{
//		return base.remoteCheckNewCommads();
//	}
//}

// создание списка команд
//void REMOTE_COMMANDS_old::Remote::createListCommands()
//{
//	// если есть новые команды создаем список с текущими командами
//	if (!chekNewCommand()) return;
//
//	// формируем лист команд
//	SQL_REQUEST::SQL base;
//	if (base.isConnectedBD())
//	{
//		base.createListRemoteCommands(list_commads);
//	}
//}

// отработка команд
//void REMOTE_COMMANDS_old::Remote::startCommand()
//{
//	if (!getCountCommand()) return;
//
//	for (const auto &list : list_commads) {
//		SQL_REQUEST::SQL base;
//		if (base.isConnectedBD())
//		{
//			base.startRemoteCommand(list.id,list.sip,list.command,list.user_id);
//
//			// проверяем успешно ли выполнили команду
//			if (remoteCommandChekedExecution(list.command))
//			{
//				//SQL_REQUEST::SQL base;
//				if (base.isConnectedBD()) 
//				{
//					// устанавливаем logging
//					LOG_old::Logging_old log;
//					log.createLog(list.command, list.id);
//
//					// обновляем текущий статус оператора					
//				
//					// новый статус оператора
//					remote::EStatus status;
//					
//					
//					// ВАЖНО!! адекватно работает только if else .. т.к. в кострукции swith case не находится почему то параметр!!!
//					if (list.command == remote::ECommand::AddQueue5000) {
//						status = remote::EStatus::Available;							
//					}
//					else if (list.command == remote::ECommand::AddQueue5050) {
//						status = remote::EStatus::Available;
//					}
//					else if (list.command == remote::ECommand::AddQueue5000_5050) {
//						status = remote::EStatus::Available;
//					}
//					else if (list.command == remote::ECommand::DelQueue5000) {
//						status = remote::EStatus::Available;
//					} 
//					else if (list.command == remote::ECommand::DelQueue5050)	{
//						status = remote::EStatus::Available;
//					}
//					else if (list.command == remote::ECommand::DelQueue5000_5050)	{
//						status = remote::EStatus::Available;
//					}
//					else if (list.command == remote::ECommand::Home)	{
//						status = remote::EStatus::Home;
//					}
//					else if (list.command == remote::ECommand::Exodus)	{
//						status = remote::EStatus::Exodus;
//					}
//					else if (list.command == remote::ECommand::Break)	{
//						status = remote::EStatus::Break;
//					}
//					else if (list.command == remote::ECommand::Dinner)	{
//						status = remote::EStatus::Dinner;
//					}
//					else if (list.command == remote::ECommand::Postvyzov)	{
//						status = remote::EStatus::Postvyzov;
//					}
//					else if (list.command == remote::ECommand::Studies)	{
//						status = remote::EStatus::Studies;
//					}
//					else if (list.command == remote::ECommand::IT)
//					{
//						status = remote::EStatus::It;
//					}
//					else if (list.command == remote::ECommand::Transfer)
//					{
//						status = remote::EStatus::Transfer;
//					}
//					else if (list.command == remote::ECommand::Reserve)
//					{
//						status = remote::EStatus::Reserve;
//					}
//					else if (list.command == remote::ECommand::Callback)
//					{
//						status = remote::EStatus::Callback;
//					}
//					
//					base.updateStatusOperators(list.user_id, status); 
//
//					// удаляем из БД
//					base.deleteRemoteCommand(list.id);
//				}
//			}			
//		}
//	}
//}

// кол-во команд которые на данный момент есть в памяти
//unsigned REMOTE_COMMANDS_old::Remote::getCountCommand() const
//{
//	return static_cast<unsigned>(list_commads.size());
//}

Status::Status()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_dispether(CONSTANTS::TIMEOUT::OPERATOR_STATUS)
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
	
	const std::string query = "select id,sip,command,user_id,delay from remote_commands where error = '0' ";

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
				case 2: command.command = getRemoteCommand(std::stoi(row[i]));		break;
				case 3: command.userId = std::stoi(row[i]);							break;
				case 4: command.delay  = (std::stoi(row[i]) == 1 ? true : false);	break;
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
	// найдем команду (add\del)
	ECommandType commandType = GetCommandType(_command);
	
	// такого быть не должно но все же
	if (commandType == ECommandType::Unknown) 
	{
		_errorDesciption = StringFormat("remote command %s not support",EnumToString<ECommand>(_command.command).c_str());
		return false;
	}
	
	std::string rawCommandStr;	// команда по умолчанию на отправку

	switch (commandType)
	{
		case(ECommandType::Add): rawCommandStr = COMMAND_ADD_QUEUE;	break;
		case(ECommandType::Del): rawCommandStr = COMMAND_DEL_QUEUE;	break;	
		default:		
			return false;
	}

	// найдем очередь
	EQueueNumber queue = GetQueueNumber(_command.command);

	// такого не должно быть но все же
	if (queue == EQueueNumber::Unknown)
	{
		// TODO в лог не забыть писать
		return false;
	}

	std::string request;

	// для двойной очереди или для входа в статус не связанный с очередью 
	if (queue == EQueueNumber::e5000_e5050)
	{
		for (size_t i = 1; i < 3; ++i)
		{
			EQueueNumber queue = static_cast<EQueueNumber>(i);

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
	m_log.ToBase(_command, _errorDesciption);

	// обновим текущий статус оператора
	if (!UpdateNewStatus(_command, _errorDesciption))
	{
		// TODO запись в лог что неуспешно, поставить в лог strErr в БД инфо об ошибки чтобы показать пользаку
		return false;
	}

	return true;
}

std::string Status::CreateCommand(const Command &_command, const EQueueNumber _queue, const std::string &_rawCommand)
{
	std::string request = _rawCommand;

	// сформируем строку (номер очереди)
	ReplaceResponseStatus(request, "%queue", EnumToString<EQueueNumber>(_queue));

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
		// todo в лог писать
		printf("%s", error.c_str());
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
		m_sql->Disconnect();
		// TODO тут запись в лог
		printf("%s", error.c_str());		
	}

	m_sql->Disconnect();	
}

bool Status::SendCommand(ECommandType commandType, std::string &_request, std::string &_errorDesciption)
{
	m_register.DeleteRawAll(); // очистим все текущие данные 

	if (!m_register.CreateData(_request, _errorDesciption))
	{
		return false;
	}	

	return CheckSendingCommand(commandType, _errorDesciption);
}

// проверка успешно ли выполнена команда
bool Status::CheckSendingCommand(ECommandType _commandType, std::string &_errorDesciption)
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
		case remote::ECommandType::Unknown:
			_errorDesciption = StringFormat("command %s not found", EnumToString<ECommandType>(_commandType).c_str());
			break;
		case remote::ECommandType::Del:
			if ((line.find("Removed") != std::string::npos) || (line.find("Not there") != std::string::npos)) 
			{
				status = true;
			}
			break;
		case remote::ECommandType::Add:
			if ((line.find("Added") != std::string::npos) || (line.find("Already there") != std::string::npos)) 
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
	case ECommand::AddQueue5000:
	case ECommand::AddQueue5050:	
	case ECommand::AddQueue5000_5050:		
		status = EStatus::Available;
		break;

	case ECommand::DelQueue5000:		
	case ECommand::DelQueue5050:		
	case ECommand::DelQueue5000_5050:
		status = EStatus::Unknown;
		break;		
	
	case ECommand::Home:		status = EStatus::Home;		break;
	case ECommand::Exodus:		status = EStatus::Exodus;	break;	
	case ECommand::Break:		status = EStatus::Break;	break;
	case ECommand::Dinner:		status = EStatus::Dinner;	break;
	case ECommand::Postvyzov:	status = EStatus::Postvyzov; break;
	case ECommand::Studies:		status = EStatus::Studies;	break;
	case ECommand::IT:			status = EStatus::It;		break;
	case ECommand::Transfer:	status = EStatus::Transfer; break;
	case ECommand::Reserve:		status = EStatus::Reserve;	break;
	case ECommand::Callback:	status = EStatus::Callback; break;
	
	default:
		status = EStatus::Unknown;
		break;
	}		

	const std::string query = "update operators set status = '" + std::to_string(static_cast<int>(status)) 
																+ "' where user_id = '" + std::to_string(_command.userId) + "'";

	
	if (!m_sql->Request(query, _errorDesciption))
	{
		m_sql->Disconnect();
		// TODO тут запись в лог
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

// поиск какая команда пришла
ECommandType Status::GetCommandType(const Command &_command)
{	
	// TODO может в будущем потом на std::map и static переделать
	
	switch (_command.command)
	{
		case ECommand::Enter:
		case ECommand::Exit:
		case ECommand::AuthError:
		case ECommand::ExitForce: 
						return ECommandType::Unknown;				
		
		case ECommand::AddQueue5000:
		case ECommand::AddQueue5050:
		case ECommand::AddQueue5000_5050: 
						return ECommandType::Add;
		
		case ECommand::DelQueue5000: 
		case ECommand::DelQueue5050:
		case ECommand::DelQueue5000_5050:	
		case ECommand::Available:
		case ECommand::Home: 
		case ECommand::Exodus: 
		case ECommand::Break:
		case ECommand::Dinner: 
		case ECommand::Postvyzov: 
		case ECommand::Studies: 
		case ECommand::IT: 
		case ECommand::Transfer:
		case ECommand::Reserve:
		case ECommand::Callback: 
						return ECommandType::Del;
	
	default:
		return ECommandType::Unknown;
	}	
}

// поиск номера очереди
EQueueNumber Status::GetQueueNumber(const ECommand &_command)
{
	switch (_command)
	{	
		case ECommand::AddQueue5000:
		case ECommand::DelQueue5000:
			return EQueueNumber::e5000;

		case ECommand::AddQueue5050:
		case ECommand::DelQueue5050:
			return EQueueNumber::e5050;

		case ECommand::AddQueue5000_5050:
		case ECommand::DelQueue5000_5050:		
		case ECommand::Home:
		case ECommand::Exodus:
		case ECommand::Break:
		case ECommand::Dinner:
		case ECommand::Postvyzov:
		case ECommand::Studies:
		case ECommand::IT:
		case ECommand::Transfer:
		case ECommand::Reserve:
		case ECommand::Callback:
			return EQueueNumber::e5000_e5050;
	
	default:
			return EQueueNumber::Unknown;
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
		// TODO в лог наверно хз подумать
		printf("%s", error.c_str());
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
					// пропускаем команду, т.к. оператор еще разговаривает
					continue;
				}				
			}
						
			if (!ExecuteCommand(command, error))
			{
				// TODO в лог наверно хз подумать
				printf("%s", error.c_str());

				
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
