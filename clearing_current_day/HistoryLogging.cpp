#include "HistoryLogging.h"
#include "../InternalFunction.h"
#include "../Constants.h"

using namespace utils;

HistoryLogging::HistoryLogging()
	: m_log(CONSTANTS::LOG::HISTORY_LOGGING)
{
}

HistoryLogging::~HistoryLogging()
{
}

void HistoryLogging::Execute()
{
	// получим данные
	if (!Get() || !IsExistData())
	{
		return;
	}

	std::string info = StringFormat("Clear table logging. Fields count = %u", Count());

	//m_log.ToPrint(info);
	m_log.ToFile(ELogType::Info, info);

	int errorCount = 0;
	int successCount = 0;

	for (const auto &field : m_history)
	{
		std::string error;

		if (Insert(field, error))
		{
			Delete(field.id, ECheckInsert::Yes);
			successCount++;
		}
		else
		{
			errorCount++;
			m_log.ToFile(ELogType::Error, error);
		}

		// success or error
		//m_log.ToPrint(error);
	}

	if (Count() == 0)
	{
		return;
	}

	info = StringFormat("Success = %u Error = %u", successCount, errorCount);
	m_log.ToFile(ELogType::Info, info);
}

bool HistoryLogging::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	// перед вставкой проверим есть ли такая запись в history_logging чтобы 2ой раз ее не добавлять
	if (CheckInsert(_field.id))
	{
		// запись в history_logging есть значит ее удаляем из таблицы ivr
		_errorDescription = StringFormat("logging %d is exist in table history_logging %s %s %s",
																						_field.id,
																						_field.date_time.c_str(),
																						_field.ip.c_str(),
																						_field.user_login_pc.c_str());

		Delete(_field.id, ECheckInsert::No);
		return false;
	}

	const std::string query = "insert into history_logging (id,ip,user_id,user_login_pc,pc,date_time,action) values ('" + 
																	std::to_string(_field.id) +
															"','" + _field.ip +
															"','" + std::to_string(_field.user_id) +
															"','" + _field.user_login_pc +
															"','" + _field.pc +
															"','" + _field.date_time +
															"','" + std::to_string(_field.action) + "')";


	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("query -> %s", query);
		m_log.ToFile(ELogType::Error, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();
	_errorDescription = StringFormat("logging %d sucessfully inserted %s %s %s",
																_field.id,
																_field.date_time.c_str(),
																_field.ip.c_str(),
																_field.user_login_pc.c_str());
	
	return true;
}

void HistoryLogging::Delete(int _id, ECheckInsert _check)
{
	if (_check == ECheckInsert::Yes)
	{
		if (!CheckInsert(_id))
		{
			return;
		}
	}

	const std::string query = "delete from logging where id = '" + std::to_string(_id) + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);
	}

	m_sql->Disconnect();
}

bool HistoryLogging::Get()
{
	m_history.clear();

	const std::string query = "select * from logging where date_time < '" + GetCurrentStartDay() + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Table field;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
			case 0:	field.id = std::atoi(row[i]);		break;	// id
			case 1:	field.ip = row[i];					break;	// ip
			case 2:	field.user_id = std::atoi(row[i]);	break;	// user_id
			case 3:	field.user_login_pc = row[i];		break;	// user_login_pc
			case 4:	field.pc = row[i];					break;	// pc			
			case 5:	field.date_time = row[i];			break;	// date_time
			case 6:	field.action = std::atoi(row[i]);	break;	// action			
			}
		}			

		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistoryLogging::IsExistData()
{
	return !m_history.empty();
}

bool HistoryLogging::CheckInsert(int _id)
{
	std::string error;
	const std::string query = "select count(id) from history_logging where id = '" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery -> %s", query);
		m_log.ToFile(ELogType::Error, error);

		m_sql->Disconnect();
		// ошибка считаем что нет записи
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existField;
	(std::stoi(row[0]) == 0 ? existField = false : existField = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existField;
}

int HistoryLogging::Count()
{
	return m_history.size();
}
