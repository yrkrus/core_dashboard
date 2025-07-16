#include <vector>
#include "Log.h"
#include "SQLRequest.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "RemoteCommands.h"
#include "utils.h"

using namespace utils;
using remote::ECommand;
using remote::CommandSendInfoUser;

// �������� ����
//void LOG_old::Logging_old::createLog(remote::ECommand command, int base_id)
//{
//	SQL_REQUEST::SQL base;
//
//	// ��������� 
//	if (base.isConnectedBD())
//	{
//		base.addLog(command, base_id);
//	}
//}



void LOG_old::LogToFile_old::add(std::string message)
{
	std::lock_guard<std::mutex> lock(mutex);	
	std::string mess = getCurrentDateTime() + "\t" + ELogType_to_string(current_type) + "\t" + message + "\n";

	if (file_log->is_open()) 
	{		
		*file_log << mess;
	}	
}


//void LOG::LogToFile_old::add(const std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> onhold, const std::vector<ACTIVE_SIP_old::Operators_old> *operators)
//{
//	std::lock_guard<std::mutex> lock(mutex);
//
//	std::ostringstream buffer;
//
//	buffer << "========================   " + getCurrentDateTime() + "   ==========================\n";
//
//	unsigned int countHoldBase{ 0 };
//	unsigned int countOperatorsOnHoldAsterisk{ 0 };
//
//	countHoldBase = onhold->size();
//
//	for (const auto &list : *operators)
//	{
//		if (list.isOnHold) ++countOperatorsOnHoldAsterisk;
//	}
//	buffer << "count OnHold Operators Base -> \t"			<< std::to_string(countHoldBase) << "\n";
//	buffer << "count OnHold Asterisk -> \t"					<< std::to_string(countOperatorsOnHoldAsterisk) << "\n";
//
//	if (file_log->is_open())
//	{
//		*file_log << buffer.str();
//	}
//}

//void LOG::LogToFile_old::add(const std::vector<ACTIVE_SIP_old::OnHold_old> *onhold)
//{
//	std::lock_guard<std::mutex> lock(mutex);
//	
//	std::ostringstream buffer;
//
//	buffer << "========================" + getCurrentDateTime() + "   ==========================\n";
//	
//	unsigned int countHold{ 0 };
//
//	for (const auto &list : *onhold) {
//		if (list.isOnHold) ++countHold;		
//	}
//	buffer << "count OnHold -> " << std::to_string(countHold) << "\n";
//
//	if (file_log->is_open())
//	{
//		*file_log << buffer.str();
//	}
//}

LOG_old::LogToFile_old::LogToFile_old(ELogType_old type)
{
	this->file_log = new std::ofstream;
	current_type = type;

	switch (type)
	{
		case eLogType_DEBUG: {				
			file_log->open(CONSTANTS::cFileLogDEBUG, std::ios_base::app);

			break;
		}
		case eLogType_INFO: {
			file_log->open(CONSTANTS::cFileLogINFO, std::ios_base::app);
			break;
		}
		case eLogType_ERROR: {
			file_log->open(CONSTANTS::cFileLogERROR, std::ios_base::app);
			break;
		}
	}	
	
	if (file_log->is_open()) {
		file_log->seekp(std::ios_base::end); // �������� �������� ����
	}
	else {
		switch (type)
		{
			case eLogType_DEBUG:
			{
				std::cerr << getCurrentDateTime() << " ������ ��� �������� ���-�����: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
			case eLogType_INFO:
			{
				std::cerr << getCurrentDateTime() << " ������ ��� �������� ���-�����: " << CONSTANTS::cFileLogINFO << "\n";
				break;
			}
			case eLogType_ERROR:
			{
				std::cerr << getCurrentDateTime() << " ������ ��� �������� ���-�����: " << CONSTANTS::cFileLogDEBUG << "\n";
				break;
			}
		}
	}	
}

std::string LOG_old::LogToFile_old::ELogType_to_string(const ELogType_old &elogtype)
{
	switch (elogtype)
	{
		case LOG_old::eLogType_DEBUG: {
			return "DEBUG";
			break;
		}		
		case LOG_old::eLogType_INFO: {
			return "INFO";
			break;
		}		
		case LOG_old::eLogType_ERROR: {
			return "ERROR";
			break;
		}		
	}
}

LOG_old::LogToFile_old::~LogToFile_old()
{
	if (file_log->is_open()) 
	{
		file_log->close();		
	}
	delete file_log;
}

bool Log::GetCommandInfoUser(CommandSendInfoUser &_userInfo, unsigned int _id, std::string &_errorDescription)
{
	// ������ ��� ������ �� ������������ 
	const std::string query = "select sip,ip,user_id,user_login_pc,pc from remote_commands where id = '" + std::to_string(_id) + "' limit 1";

	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		// TODO ��� ����� � ��� ������
		return false;
	}

	// ���������
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

Log::Log()
	: m_sql(std::make_shared<ISQLConnect>(false))
{
}

Log::~Log()
{
}

void Log::ToBase(remote::Command _command, std::string &_errorDescription)
{
	CommandSendInfoUser userInfo;
	if (!GetCommandInfoUser(userInfo, _command.id, _errorDescription))
	{
		_errorDescription = StringFormat("not found field %u remote command %s", _command.id, EnumToString<ECommand>(_command.command).c_str());
		return;
	}	

	// ���������� � ��� ��
	std::string query = "insert into logging (ip,user_id,user_login_pc,pc,action) values ('" + userInfo.ip +
																					   "','" + std::to_string(userInfo.id) +
																					   "','" + userInfo.user_login_pc +
																					   "','" + userInfo.pc +
																					   "','" + std::to_string(static_cast<int>(_command.command)) + "')";
	if (!m_sql->Request(query, _errorDescription))
	{
		m_sql->Disconnect();
		// TODO ��� ����� � ��� ������
		return;
	}

	m_sql->Disconnect();
}

bool Log::ToFile(ELogType _type, const std::string &_request)
{
	return false;
}
