#include "RemoteCommands.h"
#include "SQLRequest.h"
#include "InternalFunction.h"

using namespace INTERNALFUNCTION;

// construct
REMOTE_COMMANDS_old::Remote::Remote()
{	
	// формируем лист команд при 
	createListCommands();

}

bool REMOTE_COMMANDS_old::Remote::chekNewCommand()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		return base.remoteCheckNewCommads();
	}
}

// создание списка команд
void REMOTE_COMMANDS_old::Remote::createListCommands()
{
	// если есть новые команды создаем список с текущими командами
	if (!chekNewCommand()) return;

	// формируем лист команд
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		base.createListRemoteCommands(list_commads);
	}
}

// отработка команд
void REMOTE_COMMANDS_old::Remote::startCommand()
{
	if (!getCountCommand()) return;

	for (const auto &list : list_commads) {
		SQL_REQUEST::SQL base;
		if (base.isConnectedBD())
		{
			base.startRemoteCommand(list.id,list.sip,list.command,list.user_id);

			// проверяем успешно ли выполнили команду
			if (remoteCommandChekedExecution(list.command))
			{
				//SQL_REQUEST::SQL base;
				if (base.isConnectedBD()) 
				{
					// устанавливаем logging
					LOG_old::Logging log;
					log.createLog(list.command, list.id);

					// обновляем текущий статус оператора					
				
					// новый статус оператора
					remote::ecStatusOperator status;
					
					
					// ВАЖНО!! адекватно работает только if else .. т.к. в кострукции swith case не находится почему то параметр!!!
					if (list.command == remote::ecCommand::AddQueue5000) {
						status = remote::ecStatusOperator::ecAvailable;							
					}
					else if (list.command == remote::ecCommand::AddQueue5050) {
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == remote::ecCommand::AddQueue5000_5050) {
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == remote::ecCommand::DelQueue5000) {
						status = remote::ecStatusOperator::ecAvailable;
					} 
					else if (list.command == remote::ecCommand::DelQueue5050)	{
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == remote::ecCommand::DelQueue5000_5050)	{
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == remote::ecCommand::Home)	{
						status = remote::ecStatusOperator::ecHome;
					}
					else if (list.command == remote::ecCommand::Exodus)	{
						status = remote::ecStatusOperator::ecExodus;
					}
					else if (list.command == remote::ecCommand::Break)	{
						status = remote::ecStatusOperator::ecBreak;
					}
					else if (list.command == remote::ecCommand::Dinner)	{
						status = remote::ecStatusOperator::ecDinner;
					}
					else if (list.command == remote::ecCommand::Postvyzov)	{
						status = remote::ecStatusOperator::ecPostvyzov;
					}
					else if (list.command == remote::ecCommand::Studies)	{
						status = remote::ecStatusOperator::ecStudies;
					}
					else if (list.command == remote::ecCommand::IT)
					{
						status = remote::ecStatusOperator::ecIt;
					}
					else if (list.command == remote::ecCommand::Transfer)
					{
						status = remote::ecStatusOperator::ecTransfer;
					}
					else if (list.command == remote::ecCommand::Reserve)
					{
						status = remote::ecStatusOperator::ecReserve;
					}
					else if (list.command == remote::ecCommand::Callback)
					{
						status = remote::ecStatusOperator::ecCallback;
					}
					
					base.updateStatusOperators(list.user_id, status); 

					// удаляем из БД
					base.deleteRemoteCommand(list.id);
				}
			}			
		}
	}
}

// кол-во команд которые на данный момент есть в памяти
unsigned REMOTE_COMMANDS_old::Remote::getCountCommand() const
{
	return static_cast<unsigned>(list_commads.size());
}

remote::Status::Status()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_dispether(CONSTANTS::TIMEOUT::OPERATOR_STATUS)
{
}

remote::Status::~Status()
{
}

void remote::Status::Start() 
{
	//m_dispether.Start();
}

void remote::Status::Stop()
{
	m_dispether.Stop();
}

bool remote::Status::IsExistCommand()
{
	return !m_commandList.empty() ? true : false;
}

// получение новых команд из БД
bool remote::Status::GetCommand(std::string &_errorDesciption)
{
	_errorDesciption.clear();
	if (!m_commandList.empty()) 
	{
		m_commandList.clear();
	}
	
	const std::string query = "select id,sip,command,user_id from remote_commands where error = '0' ";

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
				case 0: command.id = std::stoi(row[i]);							break;
				case 1: command.sip = row[i];									break;
				case 2: command.command = getRemoteCommand(std::stoi(row[i]));	break;
				case 3: command.userId = std::stoi(row[i]);						break;
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
bool remote::Status::ExecuteCommand(const Command &_command, std::string &_errorDesciption)
{
	ecCommandType commandType = GetCommandType(_command);

	bool status = false;
	_errorDesciption = "command not found"; // default

	switch (commandType)
	{
	case remote::ecCommandType::Unknown:		
		return false;	
	
	case remote::ecCommandType::Del:
		if (!Del(_command, _errorDesciption)) 
		{
			status = false;
			break;
		}
		status = true;
		break;
	
	case remote::ecCommandType::Add:
		if (!Add(_command, _errorDesciption))
		{
			status = false;
			break;
		}
		status = true;
		break;
	
	default:
		status = false;
		break;
	}
	
	return status;
}

// выполнение команды (добавление)
bool remote::Status::Add(const Command &_command, std::string &_errorDesciption)
{
	_errorDesciption.clear();
	
	std::string request = COMMAND_ADD_QUEUE;
	ecQueueNumber queue = GetQueueNumber(_command);

	// заменяем %queue на номер очереди
	std::string repl = "%queue";
	/*size_t position = request.find(repl);
	request.replace(position, repl.length(), EnumToString<ecQueueNumber>(queue));*/

	//std::string error;
	//m_register.DeleteRawAll(); // очистим все текущие данные 

	//if (!m_queue.CreateData(request, error))
	//{
	//	// TODO тут подумать что делать (пока след. очередь)
	//	continue;
	//}



	//m_register


	return false;
}

// выполнение команды (удаление)
bool remote::Status::Del(const Command &_command, std::string &_errorDesciption)
{
	_errorDesciption.clear();

	return false;
}

// не удачное выполнение команды
void remote::Status::ExecuteCommandFail(const Command &_command, const std::string &_errorStr)
{
	std::string error;	
	const std::string query = "update remote_commands set error = '1', error_str = '"+_errorStr
											+"'  where id = '" + std::to_string(_command.id)
											+"' and sip = '" + _command.sip + "'";

	if (!m_sql->Request(query, error))
	{
		m_sql->Disconnect();
		printf("%s", error.c_str());		
	}

	m_sql->Disconnect();	
}

// поиск какая команда пришла
remote::ecCommandType remote::Status::GetCommandType(const Command &_command)
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
ecQueueNumber remote::Status::GetQueueNumber(const Command &_command)
{
	switch (_command.command) 
	{	
		case ecCommand::AddQueue5000:
		case ecCommand::DelQueue5000:
			return ecQueueNumber::e5000;

		case ecCommand::AddQueue5050:
		case ecCommand::DelQueue5050:
			return ecQueueNumber::e5050;

		case ecCommand::AddQueue5000_5050:
		case ecCommand::DelQueue5000_5050:
			return ecQueueNumber::e5000_e5050;
	
	default:
			return ecQueueNumber::Unknown;
	}
}

// выполнение команд
void remote::Status::Execute()
{
	std::string error;
	if (!GetCommand(error)) 
	{
		printf("%s", error.c_str());
		return;
	}
	
	// выполнение команд
	if (IsExistCommand()) 
	{
		for (const auto &command : m_commandList)
		{
			std::string error;
			if (!ExecuteCommand(command, error))
			{
				printf("%s", error.c_str());

				// инфо в БД что не успешно выполнили команду, дальше в gui это отображается у пользователя
				ExecuteCommandFail(command, error);
				continue;
			}
		}
	}	
}
