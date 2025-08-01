#include <vector>
#include "Log.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "RemoteCommands.h"
#include "utils.h"

using namespace utils;

bool Log::GetCommandInfoUser(CommandSendInfoUser &_userInfo, unsigned int _id, std::string &_errorDescription)
{
	// найдем все данные по пользователю 
	const std::string query = "select sip,ip,user_id,user_login_pc,pc from remote_commands where id = '" + std::to_string(_id) + "' limit 1";

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query.c_str());
		ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();		
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;	

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	_userInfo.sip			= row[i]; break;
				case 1:	_userInfo.ip			= row[i]; break;
				case 2: _userInfo.id			= std::stoi(row[i]); break;
				case 3:	_userInfo.user_login_pc = row[i]; break;
				case 4: _userInfo.pc			= row[i]; break;
			}			
		}
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return _userInfo.check();
}

void Log::OpenLogFile()
{
	// Открываем файл один раз, проверяем успех:
	m_file.open(m_name, std::ios::out | std::ios::app);
	if (!m_file.is_open())
	{
		std::cerr << GetCurrentDateTime() << " ERROR: cannot open log file '" << m_name << "'\n";
		m_ready = false;
	}
	else
	{
		m_ready = true;		
	}
}

void Log::CloseLogFile()
{
	if (m_file.is_open())
	{
		m_file.flush();
		m_file.close();
	}

	m_ready = false;
}

bool Log::IsReady() const
{
	return m_ready;
}

Log::Log(const std::string &_name)
	: m_sql(std::make_shared<ISQLConnect>(false))
	, m_name(_name)
	, m_ready(false)
{
	OpenLogFile();
}

Log::~Log()
{
	CloseLogFile();	
}

void Log::ToBase(Command _command)
{
	std::string error;
	CommandSendInfoUser userInfo;
	if (!GetCommandInfoUser(userInfo, _command.id, error))
	{
		error += METHOD_NAME;
		error += StringFormat("not found field %u remote command %s", _command.id, EnumToString<ECommand>(_command.command).c_str());
		ToFile(ELogType::Error, error);
		return;
	}	

	// записываем в лог БД
	std::string query = "insert into logging (ip,user_id,user_login_pc,pc,action) values ('" + userInfo.ip +
																					   "','" + std::to_string(userInfo.id) +
																					   "','" + userInfo.user_login_pc +
																					   "','" + userInfo.pc +
																					   "','" + std::to_string(static_cast<int>(_command.command)) + "')";
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query.c_str());
		ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();
}

void Log::ToFile(ELogType _type, const std::string &_message)
{
	
	if (!IsReady())
	{
		printf("Log file %s is not open!\n", m_name.c_str());
		return;
	}
	
	std::string message = GetCurrentDateTime() + "\t" + EnumToString<ELogType>(_type) + "\t" + _message + "\n";		
		
	std::lock_guard<std::mutex> lock(m_mutex);
	m_file << message;
	m_file.flush();
}

void Log::ToPrint(const std::string &_message)
{
	printf("%s\n", _message.c_str());
	fflush(stdout);
}
