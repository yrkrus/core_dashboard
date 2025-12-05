#include "HistoryQueueLisa.h"
#include "../utils/InternalFunction.h"
#include "../system/Constants.h"

using namespace utils;

HistoryQueueLisa::HistoryQueueLisa()
	: m_log(std::make_shared<Log>(CONSTANTS::LOG::HISTORY_QUEUE_LISA))
{
}

HistoryQueueLisa::~HistoryQueueLisa()
{
}

bool HistoryQueueLisa::Execute()
{
	// получим данные
	if (!Get() || !IsExistData())
	{
		return true;
	}

	std::string info = StringFormat("Clear table queue_lisa. Fields count = %u", Count());

	m_log->ToPrint(info);
	m_log->ToFile(ecLogType::eInfo, info);

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
			m_log->ToFile(ecLogType::eError, error);
		}

		// success or error
		m_log->ToPrint(error);
	}

	if (Count() == 0)
	{
		return true;
	}

	info = StringFormat("Success = %u Error = %u", successCount, errorCount);
	m_log->ToPrint(info);

	m_log->ToFile(ecLogType::eInfo, info);

	return (errorCount != 0 ? false :  true); 
}

bool HistoryQueueLisa::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	if (CheckInsert(_field.id))
	{
	 // запись в history_queue есть значит ее удаляем из таблицы queue
		_errorDescription = StringFormat("queue_lisa %d is exist in table history_queue %s %s %d",
																				_field.id,								
																				_field.date_time.c_str(),
																				_field.phone.c_str(),
																				_field.talk_time);


		Delete(_field.id, ECheckInsert::No);
		return false;
	}
	
	std::string query;
		
	query = "insert into history_queue_lisa (id,phone,date_time,talk_time,call_id,to_queue,answered,hash) values ('" + std::to_string(_field.id) +
						"','" + _field.phone +
						"','" + _field.date_time +
						"','" + std::to_string(_field.talk_time) +
						"','" + _field.call_id +
						"','" + std::to_string(_field.to_queue) +
						"','" + std::to_string(_field.answered) + 
						"','" + std::to_string(_field.hash) + "')";
	

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += StringFormat("%s\tquery \t%s", METHOD_NAME, query.c_str());
		m_log->ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

	_errorDescription = StringFormat("queue_lisa %d sucessfully inserted %s %s %d",
																	_field.id,
																	_field.date_time.c_str(),
																	_field.phone.c_str(),
																	_field.talk_time);

	return true;
}

void HistoryQueueLisa::Delete(int _id, ECheckInsert _check)
{	
	if (_check == ECheckInsert::Yes)
	{
		if (!CheckInsert(_id))
		{
			return;
		}
	}
	
	const std::string query = "delete from queue_lisa where id = '" + std::to_string(_id) + "'";
	
	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();	
}

bool HistoryQueueLisa::Get()
{
	m_history.clear();

	const std::string query = "select * from queue_lisa where date_time < '" + GetCurrentStartDay() + "'";

	std::string errorDescription;
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return false;
	}

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Table field;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0:	field.id			= std::atoi(row[i]);	break;	// id				
				case 1:	field.phone			= row[i];				break;	// phone			
				case 2:	field.date_time		= row[i];				break;	// date_time		
				case 3: if (row[i]) field.talk_time = std::atoi(row[i]);		break;	// talk_time
				case 4: if (row[i]) field.call_id = row[i];		    break;	// call_id                
                case 5:	field.to_queue		= to_bool(row[i]);	    break;	// to_queue
				case 6:	field.answered		= to_bool(row[i]);	    break;	// answered
				case 7:	if (row[i]) field.hash = string_to_size_t(row[i]);	break;	// hash			
			}                 
        }
		
		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistoryQueueLisa::IsExistData()
{
	return !m_history.empty();
}

bool HistoryQueueLisa::CheckInsert(int _id)
{
	std::string errorDescription;
	const std::string query = "select count(id) from history_queue_lisa where id = '" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log->ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		// ошибка считаем что нет записи
		return false;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	if (result == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_RES *result = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row == nullptr)
	{
		errorDescription = StringFormat("%s\tMYSQL_ROW row = nullptr", METHOD_NAME);
		m_log->ToFile(ecLogType::eError, errorDescription);
		m_sql->Disconnect();
		return false;
	}

	bool existField;
	(std::stoi(row[0]) == 0 ? existField = false : existField = true);

	mysql_free_result(result);
	m_sql->Disconnect();

	return existField;
}

size_t HistoryQueueLisa::Count()
{
	return m_history.size();
}
