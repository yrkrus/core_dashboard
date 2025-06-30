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

			// провер€ем успешно ли выполнили команду
			if (remoteCommandChekedExecution(list.command))
			{
				//SQL_REQUEST::SQL base;
				if (base.isConnectedBD()) 
				{
					// устанавливаем logging
					LOG_old::Logging log;
					log.createLog(list.command, list.id);

					// обновл€ем текущий статус оператора					
				
					// новый статус оператора
					remote::ecStatusOperator status;
					
					
					// ¬ј∆Ќќ!! адекватно работает только if else .. т.к. в кострукции swith case не находитс€ почему то параметр!!!
					if (list.command == LOG_old::ecStatus::Log_add_queue_5000) {
						status = remote::ecStatusOperator::ecAvailable;							
					}
					else if (list.command == LOG_old::ecStatus::Log_add_queue_5050) {
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == LOG_old::ecStatus::Log_add_queue_5000_5050) {
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == LOG_old::ecStatus::Log_del_queue_5000) {
						status = remote::ecStatusOperator::ecAvailable;
					} 
					else if (list.command == LOG_old::ecStatus::Log_del_queue_5050)	{
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == LOG_old::ecStatus::Log_del_queue_5000_5050)	{
						status = remote::ecStatusOperator::ecAvailable;
					}
					else if (list.command == LOG_old::ecStatus::Log_home)	{
						status = remote::ecStatusOperator::ecHome;
					}
					else if (list.command == LOG_old::ecStatus::Log_exodus)	{
						status = remote::ecStatusOperator::ecExodus;
					}
					else if (list.command == LOG_old::ecStatus::Log_break)	{
						status = remote::ecStatusOperator::ecBreak;
					}
					else if (list.command == LOG_old::ecStatus::Log_dinner)	{
						status = remote::ecStatusOperator::ecDinner;
					}
					else if (list.command == LOG_old::ecStatus::Log_postvyzov)	{
						status = remote::ecStatusOperator::ecPostvyzov;
					}
					else if (list.command == LOG_old::ecStatus::Log_studies)	{
						status = remote::ecStatusOperator::ecStudies;
					}
					else if (list.command == LOG_old::ecStatus::Log_IT)
					{
						status = remote::ecStatusOperator::ecIt;
					}
					else if (list.command == LOG_old::ecStatus::Log_transfer)
					{
						status = remote::ecStatusOperator::ecTransfer;
					}
					else if (list.command == LOG_old::ecStatus::Log_reserve)
					{
						status = remote::ecStatusOperator::ecReserve;
					}
					else if (list.command == LOG_old::ecStatus::Log_callback)
					{
						status = remote::ecStatusOperator::ecCallback;
					}
					
					base.updateStatusOperators(list.user_id, status); 

					// удал€ем из Ѕƒ
					base.deleteRemoteCommand(list.id);
				}
			}			
		}
	}
}

// кол-во команд которые на данный момент есть в пам€ти
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

// получение новых команд из Ѕƒ
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

// выполнение команд
void remote::Status::Execute()
{
	std::string error;
	if (!GetCommand(error)) 
	{
		printf("%s", error.c_str());
		return;
	}

	if (!IsExistCommand()) 
	{
		return;
	}

	// выполнение команд
	for (const auto &command : m_commandList) 
	{
		std::string error;
		if (!ExecuteCommand(command, error)) 
		{
			printf("%s", error.c_str());
			
			// инфо в Ѕƒ что не успешно выполнили команду, дальше в gui это отображаетс€ у пользовател€
			ExecuteCommandFail(command, error);
			continue;  
		}
	}
}
