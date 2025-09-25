#include "HistoryQueue.h"
#include "../utils/InternalFunction.h"
#include "../system/Constants.h"

using namespace utils;

HistoryQueue::HistoryQueue()
	: m_log(CONSTANTS::LOG::HISTORY_QUEUE)
{
}

HistoryQueue::~HistoryQueue()
{
}

bool HistoryQueue::Execute()
{
	// получим данные
	if (!Get() || !IsExistData())
	{
		return true;
	}

	std::string info = StringFormat("Clear table queue. Fields count = %u", Count());

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

bool HistoryQueue::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	if (CheckInsert(_field.id))
	{
	 // запись в history_queue есть значит ее удаляем из таблицы queue
		_errorDescription = StringFormat("queue %d is exist in table history_queue %d %d %s %s %s",
																				_field.id,
																				_field.sip,
																				_field.number_queue,
																				_field.date_time.c_str(),
																				_field.phone.c_str(),
																				_field.waiting_time.c_str());


		Delete(_field.id, ECheckInsert::No);
		return false;
	}
	
	std::string query;

	if (_field.sip != -1)
	{	 // на случай если нет NULL полей		
		query = "insert into history_queue (id,number_queue,phone,waiting_time,date_time,sip,talk_time,answered,fail,hash,call_id,id_ivr) values ('" + std::to_string(_field.id) +
						"','" + std::to_string(_field.number_queue) +
						"','" + _field.phone +
						"','" + _field.waiting_time +
						"','" + _field.date_time +
						"','" + std::to_string(_field.sip) +
						"','" + _field.talk_time +
						"','" + std::to_string(_field.answered) +
						"','" + std::to_string(_field.fail) +
						"','" + std::to_string(_field.hash) + 
						"','" + _field.call_id + 
						"','" + _field.id_ivr + "')";
	}
	else
	{	 // на случай если есть NULL поля

		query = "insert into history_queue (id,number_queue,phone,waiting_time,date_time,sip,answered,fail) values ('" + std::to_string(_field.id) +
			"','" + std::to_string(_field.number_queue) +
			"','" + _field.phone +
			"','" + _field.waiting_time +
			"','" + _field.date_time +
			"','" + std::to_string(_field.sip) +
			"','" + std::to_string(_field.answered) +
			"','" + std::to_string(_field.fail) + "')";
	}

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += StringFormat("%s\tquery \t%s", METHOD_NAME, query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

	_errorDescription = StringFormat("queue %d sucessfully inserted %d %d %s %s %s",
																	_field.id,
																	_field.sip,
																	_field.number_queue,
																	_field.date_time.c_str(),
																	_field.phone.c_str(),
																	_field.waiting_time.c_str());

	return true;
}

void HistoryQueue::Delete(int _id, ECheckInsert _check)
{	
	if (_check == ECheckInsert::Yes)
	{
		if (!CheckInsert(_id))
		{
			return;
		}
	}
	
	const std::string query = "delete from queue where id = '" + std::to_string(_id) + "'";
	
	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();	
}

bool HistoryQueue::Get()
{
	m_history.clear();

	const std::string query = "select * from queue where date_time < '" + GetCurrentStartDay() + "'";

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
				case 0:	field.id			= std::atoi(row[i]);	break;	// id
				case 1:	field.number_queue	= std::atoi(row[i]);	break;	// number_queue
				case 2:	field.phone			= row[i];				break;	// phone
				case 3:	field.waiting_time	= row[i];				break;	// waiting_time
				case 4:	field.date_time		= row[i];				break;	// date_time
				case 5:	field.sip			= std::atoi(row[i]);	break;	// sip
				case 6: if (row[i]) field.talk_time = row[i];		break;	// talk_time
				case 7:	field.answered		= std::atoi(row[i]);	break;	// answered
				case 8:	field.fail			= std::atoi(row[i]);	break;	// fail
				case 9:	if (row[i]) field.hash = string_to_size_t(row[i]);	break;	// hash
				case 10: if (row[i]) field.call_id = row[i];		break;	// call_id
				case 11: if (row[i]) field.id_ivr = row[i];			break;	// id_ivr
			}
		}
		
		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistoryQueue::IsExistData()
{
	return !m_history.empty();
}

bool HistoryQueue::CheckInsert(int _id)
{
	std::string error;
	const std::string query = "select count(id) from history_queue where id = '" + std::to_string(_id) + "'";

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

size_t HistoryQueue::Count()
{
	return m_history.size();
}
