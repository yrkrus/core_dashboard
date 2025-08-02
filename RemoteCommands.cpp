#include "RemoteCommands.h"
#include "Log.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "Queue.h"
#include "utils.h"

using namespace utils;


Status::Status()
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_dispether(CONSTANTS::TIMEOUT::OPERATOR_STATUS)
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

// ��������� ����� ������ �� ��
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

	// ���������
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
			}				
		}

		// ������� ��������� �������
		m_commandList.push_back(command);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}

// ���������� �������
bool Status::ExecuteCommand(const Command &_command, std::string &_errorDesciption)
{	
	// ������ ������� (add\del)
	ECommandType commandType = GetCommandType(_command);
	
	// ������ ���� �� ������ �� ��� ��
	if (commandType == ECommandType::Unknown) 
	{
		_errorDesciption = StringFormat("remote command %s not support",EnumToString<ECommand>(_command.command).c_str());
		return false;
	}
	
	std::string rawCommandStr;	// ������� �� ��������� �� ��������

	switch (commandType)
	{
		case(ECommandType::Add): rawCommandStr = COMMAND_ADD_QUEUE;	break;
		case(ECommandType::Del): rawCommandStr = COMMAND_DEL_QUEUE;	break;	
		default:		
			return false;
	}

	// ������ �������
	EQueueNumber queue = GetQueueNumber(_command.command);

	// ������ �� ������ ���� �� ��� ��
	if (queue == EQueueNumber::Unknown)
	{
		// TODO � ��� �� ������ ������
		return false;
	}

	std::string request;

	// ��� ������� ������� ��� ��� ����� � ������ �� ��������� � �������� 
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

	// ������� � ��� ������
	m_log->ToBase(_command);

	// ������� ������� ������ ���������
	if (!UpdateNewStatus(_command, _errorDesciption))
	{
		// TODO ������ � ��� ��� ���������, ��������� � ��� strErr � �� ���� �� ������ ����� �������� ��������
		return false;
	}

	return true;
}

std::string Status::CreateCommand(const Command &_command, const EQueueNumber _queue, const std::string &_rawCommand)
{
	std::string request = _rawCommand;

	// ���������� ������ (����� �������)
	ReplaceResponseStatus(request, "%queue", EnumToString<EQueueNumber>(_queue));

	// ���������� ������ (sip �������)
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
		m_log->ToFile(ELogType::Error, error);
	}

	m_sql->Disconnect();	
}

// �� ������� ���������� �������
void Status::ExecuteCommandFail(const Command &_command, const std::string &_errorStr)
{
	std::string error;	
	const std::string query = "update remote_commands set error = '1', error_str = '"+_errorStr
											+"'  where id = '" + std::to_string(_command.id)
											+"' and sip = '" + _command.sip + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		m_log->ToFile(ELogType::Error, error);

		m_sql->Disconnect();				
	}

	m_sql->Disconnect();	
}

bool Status::SendCommand(ECommandType commandType, std::string &_request, std::string &_errorDesciption)
{
	m_register.DeleteRawAll(); // ������� ��� ������� ������ 

	if (!m_register.CreateData(_request, _errorDesciption))
	{
		return false;
	}	

	return CheckSendingCommand(commandType, _errorDesciption);
}

// �������� ������� �� ��������� �������
bool Status::CheckSendingCommand(ECommandType _commandType, std::string &_errorDesciption)
{
	bool status = false;

	// �������� �����
	if (!m_register.IsExistRaw())
	{
		_errorDesciption = "not respond asterisk command";
		return false;
	}

	std::string rawLines = m_register.GetRawFirst();
	if (rawLines.empty())
	{
		// TODO ��� ��������, ��� ������!
		return true;
	}

	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))
	{
		switch (_commandType)
		{
		case ECommandType::Unknown:
			_errorDesciption = StringFormat("command %s not found", EnumToString<ECommandType>(_commandType).c_str());
			break;
		case ECommandType::Del:
			if ((line.find("Removed") != std::string::npos) || (line.find("Not there") != std::string::npos)) 
			{
				status = true;
			}
			break;
		case ECommandType::Add:
			if ((line.find("Added") != std::string::npos) || (line.find("Already there") != std::string::npos)) 
			{
				status = true;
			}
			break;
		}
	}
	// �������
	m_register.DeleteRawAll();

	return status;
}

bool Status::UpdateNewStatus(const Command &_command, std::string &_errorDesciption)
{	
	// ����� ������ ���������
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
		// TODO ��� ������ � ���
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
		// TODO � ���
		printf("%s", _errorDesciption.c_str());
		m_sql->Disconnect();
		// ������ ������� ��� ��� ������
		return false;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool exist;
	std::stoi(row[0]) == 0 ? exist = false : exist = true;

	mysql_free_result(result);
	m_sql->Disconnect();

	return exist;
}

// ����� ����� ������� ������
ECommandType Status::GetCommandType(const Command &_command)
{	
	// TODO ����� � ������� ����� �� std::map � static ����������
	
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

// ����� ������ �������
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

// ���������� ������
bool Status::Execute()
{
	std::string error;
	if (!GetCommand(error)) 
	{
		// TODO � ��� ������� �� ��������
		printf("%s", error.c_str());
		return false;
	}
	
	// ���������� ������
	if (IsExistCommand()) 
	{
		for (const auto &command : m_commandList)
		{
			std::string error;

			if (command.delay)	// ���������� ���������� �������
			{
				// �������� ������������� �� ��� �������� ��� ���
				if (IsTalkOperator(command.sip, error)) 
				{
					// ���������� �������, �.�. �������� ��� �������������
					continue;
				}				
			}
						
			if (!ExecuteCommand(command, error))
			{
				// TODO � ��� ������� �� ��������
				printf("%s", error.c_str());

				
				// �� ������ ��������� �������, ����� ������� �� ����� �.�. � ��������� �� ����� ������� �� ������
				if (command.delay) 
				{
					DeleteCommand(command);
					continue;
				}

				// ���� � �� ��� �� ������� ��������� �������, ������ � gui ��� ������������ � ������������
				ExecuteCommandFail(command, error);
				continue;
			}
			else
			{
				// ������� ������������ ������� ������� ��
				DeleteCommand(command);
			}							
		}
	}

	return true;
}
