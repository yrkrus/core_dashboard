#include "HistoryOnHold.h"
#include "../InternalFunction.h"
#include "../Constants.h"

using namespace utils;

HistoryOnHold::HistoryOnHold()
	: m_log(CONSTANTS::LOG::HISTORY_ONHOLD)
{
}

HistoryOnHold::~HistoryOnHold()
{
}

bool HistoryOnHold::Execute()
{
	// получим данные
	if (!Get() || !IsExistData())
	{
		return true;
	}

	std::string info = StringFormat("Clear operators_ohhold. Fields count = %u", Count());

	m_log.ToPrint(info);
	m_log.ToFile(ecLogType::eInfo, info);

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
			m_log.ToFile(ecLogType::eError, error);
		}

		// success or error
		m_log.ToPrint(error);
	}

	if (Count() == 0)
	{
		return true;
	}

	info = StringFormat("Success = %u Error = %u", successCount, errorCount);
	m_log.ToPrint(info);

	m_log.ToFile(ecLogType::eInfo, info);

	return (errorCount != 0 ? false :  true); 
}

bool HistoryOnHold::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	// перед вставкой проверим есть ли такая запись в history_onhold чтобы 2ой раз ее не добавлять
	if (CheckInsert(_field.id))
	{
		// запись в history_logging есть значит ее удаляем из таблицы ivr
		_errorDescription = StringFormat("operators_ohhold %d is exist in table history_onhold %d %s %s",
																								_field.id,
																								_field.sip,
																								_field.date_time_start.c_str(),
																								_field.phone.c_str());

		Delete(_field.id, ECheckInsert::No);
		return false;
	}

	const std::string query = "insert into history_onhold (id,sip,date_time_start,date_time_stop,phone) values ('" + 
																					std::to_string(_field.id) +
																			"','" + std::to_string(_field.sip) +
																			"','" + _field.date_time_start +
																			"','" + _field.date_time_stop +
																			"','" + _field.phone + "')";



	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();	
	_errorDescription = StringFormat("operators_ohhold %d sucessfully inserted %d %s %s",
																				_field.id,
																				_field.sip,
																				_field.date_time_start.c_str(),
																				_field.phone.c_str());

	return true;
}

void HistoryOnHold::Delete(int _id, ECheckInsert _check)
{
	if (_check == ECheckInsert::Yes)
	{
		if (!CheckInsert(_id))
		{
			return;
		}
	}

	const std::string query = "delete from operators_ohhold where id = '" + std::to_string(_id) + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();
}

bool HistoryOnHold::Get()
{
	m_history.clear();

	const std::string query = "select * from operators_ohhold where date_time_start < '" + GetCurrentStartDay() + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);

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
			case 1:	field.sip = std::atoi(row[i]);		break;	// ip
			case 2:	field.date_time_start = row[i];		break;	// date_time_start
			case 3:	field.date_time_stop = row[i];		break;	// date_time_stop
			case 4:	field.phone = row[i];				break;	// phone				
			}
		}	

		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistoryOnHold::IsExistData()
{
	return !m_history.empty();
}

bool HistoryOnHold::CheckInsert(int _id)
{
	std::string error;
	const std::string query = "select count(id) from history_onhold where id = '" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);

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

size_t HistoryOnHold::Count()
{
	return m_history.size();
}
