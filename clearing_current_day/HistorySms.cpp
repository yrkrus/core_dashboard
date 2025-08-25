#include "HistorySms.h"
#include "../InternalFunction.h"
#include "../Constants.h"

using namespace utils;

HistorySms::HistorySms()
	: m_log(CONSTANTS::LOG::HISTORY_SMS)
	, m_smsInfo(ecSmsInfoTable::eHistorySMS)
{
}

HistorySms::~HistorySms()
{
}

bool HistorySms::Execute()
{
	// получим данные
	if (!Get() || !IsExistData())
	{
		return true;
	}

	std::string info = StringFormat("Clear table sms_sending. Fields count = %u", Count());

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

	// пробежимся по истории смс
	m_smsInfo.Execute();

	return (errorCount != 0 ? false :  true); 
}

bool HistorySms::Insert(const Table &_field, std::string &_errorDescription)
{
	_errorDescription.clear();

	// перед вставкой проверим есть ли такая запись в history_logging чтобы 2ой раз ее не добавлять
	if (CheckInsert(_field.id))
	{
		// запись в history_logging есть значит ее удаляем из таблицы ivr
		_errorDescription = StringFormat("sms_sending %d is exist in table history_sms_sending %s %s %s %d",
																			_field.id,
																			_field.date_time.c_str(),
																			_field.phone.c_str(),
																			_field.user_login_pc.c_str(),
																			_field.count_real_sms);

		Delete(_field.id, ECheckInsert::No);
		return false;
	}

	std::string query;

	// устанавливаем данные в history_sms_sending
	if (!_field.status_date.empty())
	{
		query = "insert into history_sms_sending (id,user_id,date_time,phone,message,sms_id,status,user_login_pc,count_real_sms,sms_type,status_date) values ('" +
																				std::to_string(_field.id) +
																				"','" + std::to_string(_field.user_id) +
																				"','" + _field.date_time +
																				"','" + _field.phone +
																				"','" + _field.message +
																				"','" + std::to_string(_field.sms_id) +
																				"','" + std::to_string(_field.status) +
																				"','" + _field.user_login_pc +
																				"','" + std::to_string(_field.count_real_sms) +
																				"','" + std::to_string(_field.sms_type) +
																				"','" + _field.status_date + "')";
	}
	else  // есть null поле на status_date
	{
		query = "insert into history_sms_sending (id,user_id,date_time,phone,message,sms_id,status,user_login_pc,count_real_sms,sms_type) values ('" +
																				std::to_string(_field.id) +
																				"','" + std::to_string(_field.user_id) +
																				"','" + _field.date_time +
																				"','" + _field.phone +
																				"','" + _field.message +
																				"','" + std::to_string(_field.sms_id) +
																				"','" + std::to_string(_field.status) +
																				"','" + _field.user_login_pc +
																				"','" + std::to_string(_field.count_real_sms) +
																				"','" + std::to_string(_field.sms_type) + "')";
	}


	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();
	_errorDescription = StringFormat("sms_sending %d sucessfully inserted %s %s %s %d",
																		_field.id,
																		_field.date_time.c_str(),
																		_field.phone.c_str(),
																		_field.user_login_pc.c_str(),
																		_field.count_real_sms);	

	return true;
}

void HistorySms::Delete(int _id, ECheckInsert _check)
{
	if (_check == ECheckInsert::Yes)
	{
		if (!CheckInsert(_id))
		{
			return;
		}
	}

	const std::string query = "delete from sms_sending where id = '" + std::to_string(_id) + "'";

	std::string error;
	if (!m_sql->Request(query, error))
	{
		error += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, error);
	}

	m_sql->Disconnect();
}

bool HistorySms::Get()
{
	m_history.clear();

	const std::string query = "select * from sms_sending where date_time < '" + GetCurrentStartDay() + "'";

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
			case 1:	field.user_id = std::atoi(row[i]);	break;	// user_id			
			case 2:	field.date_time = row[i];			break;	// date_time
			case 3:	field.phone = row[i];				break;	// phone
			case 4:	field.message = row[i];				break;	// message
			case 5: field.sms_id = string_to_size_t(row[i]); break;	// sms_id
			case 6:	field.status = std::atoi(row[i]);		break;	// status					
			case 7:	field.user_login_pc = row[i];	break;	// user_login_pc
			case 8:	field.count_real_sms = std::atoi(row[i]);	break;	// count_real_sms
			case 9:	field.sms_type = std::atoi(row[i]);	break;	// sms_type
			case 10: if (row[i]) field.status_date = row[i];	break;	// status_date	
			}
		}	
		
		m_history.push_back(field);
	}

	mysql_free_result(result);
	m_sql->Disconnect();

	return true;
}


bool HistorySms::IsExistData()
{
	return !m_history.empty();
}

bool HistorySms::CheckInsert(int _id)
{
	std::string error;
	const std::string query = "select count(id) from history_sms_sending where id = '" + std::to_string(_id) + "'";

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

size_t HistorySms::Count()
{
	return m_history.size();
}
