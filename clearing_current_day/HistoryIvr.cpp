#include "HistoryIvr.h"
#include "../InternalFunction.h"
#include "../Constants.h"

using namespace utils;

HistoryIvr::HistoryIvr()
	: m_log(CONSTANTS::LOG::HISTORY_IVR)
{
}

HistoryIvr::~HistoryIvr()
{
}

bool HistoryIvr::Execute()
{
	// получим данные
	 if (!Get() || !IsExistData())
	 {
	 	return true;
	 }

	 std::string info = StringFormat("Clear table ivr. Fields count = %u", Count());
	
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

bool HistoryIvr::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	// перед вставкой проверим есть ли такая запись в history_ivr чтобы 2ой раз ее не добавлять
	if (CheckInsert(_field.id)) 
	{
		// запись в history_ivr есть значит ее удаляем из таблицы ivr
		_errorDescription = StringFormat("ivr %d is exist in table history_ivr %s %s %s", 
																				_field.id, 
																				_field.date_time.c_str(), 
																				_field.phone.c_str(), 
																				_field.waiting_time.c_str());

		Delete(_field.id, ECheckInsert::No);
		return false;
	}
	
	std::string query;
	if (!_field.phone_operator.empty() && !_field.region.empty()) 
	{
		query = "insert into history_ivr (id,phone,waiting_time,date_time,trunk,to_queue,to_robot,operator,region) values ('" + std::to_string(_field.id) +
								"','" + _field.phone +
								"','" + _field.waiting_time +
								"','" + _field.date_time +
								"','" + _field.trunk +
								"','" + std::to_string(_field.to_queue) +
								"','" + std::to_string(_field.to_robot) + 
								"','" + _field.phone_operator +
								"','" + _field.region +	"')";
	} 
	else 
	{
		query = "insert into history_ivr (id,phone,waiting_time,date_time,trunk,to_queue,to_robot) values ('" + std::to_string(_field.id) +
								"','" + _field.phone +
								"','" + _field.waiting_time +
								"','" + _field.date_time +
								"','" + _field.trunk +
								"','" + std::to_string(_field.to_queue) +
								"','" + std::to_string(_field.to_robot) + "')";
	} 
	

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription = StringFormat("%s\tquery \t%s\t%s", METHOD_NAME, query.c_str(), _errorDescription.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

	
	_errorDescription = StringFormat("ivr %d sucessfully inserted %s %s %s", 
																	_field.id, 
																	_field.date_time.c_str(), 
																	_field.phone.c_str(), 
																	_field.waiting_time.c_str());
	return true;
}

void HistoryIvr::Delete(int _id, ECheckInsert _check)
{
	if (_check == ECheckInsert::Yes) 
	{
		if (!CheckInsert(_id)) 
		{
			return;
		}
	}	
	
	const std::string query = "delete from ivr where id = '" + std::to_string(_id) + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();
}

bool HistoryIvr::Get()
{
	m_history.clear();

	const std::string query = "select * from ivr where date_time < '" + GetCurrentStartDay() + "'";

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
			case 0:	field.id = std::atoi(row[i]);	break;	// id
			case 1:	field.phone = row[i];			break;	// phone
			case 2:	field.waiting_time = row[i];	break;	// waiting_time
			case 3:	field.date_time = row[i];		break;	// date_time
			case 4:	field.trunk = row[i];			break;	// trunk			
			case 5:	field.to_queue = std::atoi(row[i]);	break;	// to_queue
			case 6:	field.to_robot = std::atoi(row[i]);	break;	// to_robot
			case 7: if (row[i]) field.phone_operator = row[i];		break;	// operator
			case 8: if (row[i]) field.region = row[i];				break;	// region			
			}
		}

		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistoryIvr::IsExistData()
{
	return !m_history.empty();
}

bool HistoryIvr::CheckInsert(int _id)
{
	std::string error;
	const std::string query = "select count(id) from history_ivr where id = '" + std::to_string(_id) + "'";

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

size_t HistoryIvr::Count()
{
	return m_history.size();
}
