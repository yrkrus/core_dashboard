#include "HistoryQueue.h"
#include "../InternalFunction.h"

using namespace utils;

HistoryQueue::HistoryQueue()
{
}

HistoryQueue::~HistoryQueue()
{
}

void HistoryQueue::Execute()
{
	// получим данные
	if (!Get() && !IsExistData())
	{
		return;
	}

	for (const auto &field : m_history) 
	{
		std::string error;

		if (!Insert(field,error))
		{
			// TODO в лог
			printf("%s", error.c_str());			
			continue;
		}

		Delete(field.id);
		printf("%u - %s\n", field.id, field.phone.c_str());
	}
}

bool HistoryQueue::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	std::string query;

	if (_field.sip != -1)
	{	 // на случай если нет NULL полей		
		query = "insert into history_queue (id,number_queue,phone,waiting_time,date_time,sip,talk_time,answered,fail,hash) values ('" + std::to_string(_field.id) +
						"','" + std::to_string(_field.number_queue) +
						"','" + _field.phone +
						"','" + _field.waiting_time +
						"','" + _field.date_time +
						"','" + std::to_string(_field.sip) +
						"','" + _field.talk_time +
						"','" + std::to_string(_field.answered) +
						"','" + std::to_string(_field.fail) +
						"','" + std::to_string(_field.hash) + "')";
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
		// TODO в лог
		printf("%s %s", METHOD_NAME, _errorDescription.c_str());
		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

	return true;
}

void HistoryQueue::Delete(int _id)
{
	const std::string query = "delete from queue where id = '" + std::to_string(_id) + "'";
	
	std::string error;
	if (!m_sql->Request(query, error))
	{
		// TODO в лог
		printf("%s", error.c_str());		
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
		// TODO в лог
		printf("%s", error.c_str());
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
