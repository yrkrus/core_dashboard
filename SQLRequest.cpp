#include "SQLRequest.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "IVR.h"
#include "HouseKeeping.h"
#
#include <mysql/mysql.h>
#include <iterator>
#include <string>
#include <memory>
#include <list>

SQL_REQUEST::SQL::SQL()
{	
	createMySQLConnect(this->mysql);
}

void SQL_REQUEST::SQL::createMySQLConnect(MYSQL &mysql)
{

	const char *host	= CONSTANTS::cHOST.c_str();
	const char *login	= CONSTANTS::cLOGIN.c_str();
	const char *pwd		= CONSTANTS::cPASSWORD.c_str();
	const char *bd		= CONSTANTS::cBD.c_str();

	if (mysql_init(&mysql) == nullptr)
	{
		// Если дескриптор не получен — выводим сообщение об ошибке
		std::cerr << "Error: can't create MySQL-descriptor\n";
		
		if (CONSTANTS::SAFE_LOG) {
			if (CONSTANTS::LOG_MODE_ERROR) {
				LOG::LogToFile log(LOG::eLogType_ERROR);
				log.add("Error: can't create MySQL-descriptor");
			}
		}
		return;
	}

	if (!mysql_real_connect(&mysql, host, login, pwd, bd, NULL, NULL, 0))
	{
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		showErrorBD("SQL_REQUEST::SQL::createMySQLConnect -> Error: can't connect to database", &mysql);
		return;
	};

	mysql_set_character_set(&mysql, "utf8");
}

// есть ли коннект с БД
bool SQL_REQUEST::SQL::isConnectedBD()
{
	// status = 0 значит коннект есть
	if (mysql_ping(&this->mysql) != 0) {
		
		createMySQLConnect(this->mysql);

		isConnectedBD();
	}
	else
	{
		return true;
	}
}

void SQL_REQUEST::SQL::query_test()
{
	if (mysql_query(&this->mysql, "SELECT * FROM ivr")) {
		std::cerr << "error query\n";
		return;
	} 


	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)))
	{
		for (int i = 0; i < num_fields; i++)
		{
			if (i > 0)
			{
				std::cout << ", ";
			}
			std::cout << row[i];
		}
		std::cout << std::endl;
	}

	mysql_free_result(result);
}

void SQL_REQUEST::SQL::insertData_test()
{
	std::string query = "insert into ivr (phone,waiting_time) values ('+79275052333','15')";

	if (mysql_query(&this->mysql, query.c_str()) != 0) {
		std::cout << "error insert\n";
	}
	else {
		std::cout << "insert ok\n";
	}

}


// добавление данных в таблицу IVR
void SQL_REQUEST::SQL::insertIVR(const char *phone, const char *time, std::string callerid)
{	
	if (!isConnectedBD()) {
		showErrorBD("SQL_REQUEST::SQL::insertIVR");
		return;
	}	
	
	// проверим есть ли такой номер 	
	if (isExistIVRPhone(phone))	{ // номер существует, обновляем данные
		
		std::string id = std::to_string(getLastIDphone(phone));
		
		updateIVR(id.c_str(), phone, time);
		return;
	}
	else {		
		std::string query = "insert into ivr (phone,waiting_time,trunk) values ('" + std::string(phone) + "','" + std::string(time) + "','" + callerid + "')";
		
		if (CONSTANTS::SAFE_LOG) {
			if (CONSTANTS::LOG_MODE_DEBUG) {
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::insertIVR(const char *phone, const char *time, std::string callerid) -> "+ query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD("SQL_REQUEST::SQL::insertIVR -> Data (insertIVR) error -> query("+query+")", &this->mysql);
		}	
	}	

	mysql_close(&this->mysql);
}

// существует ли такой уже номер в таблице IVR
bool SQL_REQUEST::SQL::isExistIVRPhone(const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistIVRPhone");
		return true;
	}
	
	const std::string query = "select count(phone) from ivr where phone = '" 
							  + std::string(phone) +"' and  date_time > '"
							  + getCurrentDateTimeAfterMinutes(2)+"' and to_queue = '0' order by date_time desc";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistIVRPhone(const char *phone) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str() ) != 0)	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistIVRPhone -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);	
	
	bool existIvrPhone;
	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;
	
	mysql_free_result(result);	

	return existIvrPhone;	
}

// получение последнего ID актуального
int SQL_REQUEST::SQL::getLastIDphone(const char *phone)
{	
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getLastIDphone");
		return -1;
	}

	const std::string query = "select id from ivr where phone = "
		+ std::string(phone) + " and date_time > '"
		+ getCurrentStartDay() + "' order by date_time desc limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::getLastIDphone(const char *phone) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что нет записи		
		showErrorBD("SQL_REQUEST::SQL::getLastIDphone -> query(" + query + ")", &this->mysql);
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	int id = std::stoi(row[0]);

	mysql_free_result(result);	

	return id;
}

// обновление данных в таблице IVR
void SQL_REQUEST::SQL::updateIVR(const char *id,const char *phone, const char *time)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateIVR");
		return;
	}

	std::string query = "update ivr set waiting_time = '" + std::string(time) + "' where phone = '" + std::string(phone) + "' and id ='"+std::string(id)+"'";
	
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateIVR(const char *id,const char *phone, const char *time) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateIVR -> Data (updateIVR) error -> query(" + query + ")",&this->mysql);
	};
	

	mysql_close(&this->mysql);
}

// добавление данных в таблицу QUEUE
void SQL_REQUEST::SQL::insertQUEUE(const char *queue, const char *phone, const char *time)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertQUEUE");
		return;
	}	


	// проверим есть ли такой номер 	
	if (isExistQUEUE(queue,phone))
	{ // номер существует, обновляем данные
	
		std::string id = std::to_string(getLastIDQUEUE(phone));
		updateQUEUE(id.c_str(),phone, time);
		return;
	}
	else
	{
		std::string query = "insert into queue (number_queue,phone,waiting_time) values ('" 
							+ std::string(queue) + "','" 
							+ std::string(phone) + "','" 
							+ std::string(time) + "')";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::insertQUEUE(const char *queue, const char *phone, const char *time) -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD("SQL_REQUEST::SQL::insertQUEUE -> Data (insertQUEUE) error -> query(" + query + ")", &this->mysql);
		}		
	}

	mysql_close(&this->mysql);

}

// существует ли такой уже номер в таблице QUEUE
bool SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE");
		return true;
	}	


	// правильней проверять сначало разговор	
	const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
		+ "' and phone = '" + std::string(phone) + "'"
		+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'"
		+ " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL order by date_time desc limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	int countPhone = std::stoi(row[0]);
	mysql_free_result(result);

	if (countPhone >= 1)
	{
		return true;
	}
	else {
		// проверяем вдруг в очереди сейчас находится звонок
		const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
			+ "' and phone = '" + std::string(phone) + "'"
			+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'" //тут типа ок, но время не затрагивается последние 15 мин
			//+ " and date_time > '" + getCurrentDateTime() + "'"
			+ " and answered ='0' and fail='0' and hash is NULL order by date_time desc limit 1";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone) -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			// ошибка считаем что есть запись		
			showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
			return true;
		}

		// результат
		MYSQL_RES *result = mysql_store_result(&this->mysql);
		MYSQL_ROW row = mysql_fetch_row(result);
		int countPhone = std::stoi(row[0]);
		
		mysql_free_result(result);

		if (countPhone >= 1)
		{
			return true; 
		}
		else {
		
			// нет разговора проверяем повтрность
			const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
				+ "' and phone = '" + std::string(phone) + "'"
				+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'" //тут типа ок, но время не затрагивается последние 15 мин
				//+ " and date_time > '" + getCurrentDateTime() + "'"
				+ " and answered ='0' and fail='1' and hash is NULL order by date_time desc limit 1";

			if (CONSTANTS::SAFE_LOG)
			{
				if (CONSTANTS::LOG_MODE_DEBUG)
				{
					LOG::LogToFile log(LOG::eLogType_DEBUG);
					log.add("SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone) -> " + query);
				}
			}

			if (mysql_query(&this->mysql, query.c_str()) != 0)
			{
				// ошибка считаем что есть запись		
				showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
				return true;
			}

			// результат
			MYSQL_RES *result = mysql_store_result(&this->mysql);
			MYSQL_ROW row = mysql_fetch_row(result);
			int countPhone = std::stoi(row[0]);
			
			mysql_free_result(result);

			if (countPhone >= 1)
			{
				return false; // считаем как новый вызов!!!
			}
			else 
			{			
			// проверка на повторность, вдруг еще раз перезвонили после того как поговорили уже	
				const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
					+ "' and phone = '" + std::string(phone) + "'"
					+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'"
					+ " and answered = '1' and fail = '0' and sip <>'-1'"
					+ " and hash is not NULL order by date_time desc limit 1";

				if (CONSTANTS::SAFE_LOG)
				{
					if (CONSTANTS::LOG_MODE_DEBUG)
					{
						LOG::LogToFile log(LOG::eLogType_DEBUG);
						log.add("SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone) -> " + query);
					}
				}

				if (mysql_query(&this->mysql, query.c_str()) != 0)
				{
					// ошибка считаем что есть запись		
					showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
					return true;
				}
				// результат
				MYSQL_RES *result = mysql_store_result(&this->mysql);
				MYSQL_ROW row = mysql_fetch_row(result);
				
				int countPhone = std::stoi(row[0]);
				mysql_free_result(result);

				if (countPhone >= 1)
				{
					return false;	// если есть запись, значит повторный звонок
				}
				
				return (countPhone == 0 ? false : true);
			}		
		}		
	}	
}

// обновление данных в таблице QUEUE
void SQL_REQUEST::SQL::updateQUEUE(const char *id, const char *phone, const char *time)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE");
		return;
	}

	std::string query = "update queue set waiting_time = '" + std::string(time) + "' where phone = '" + std::string(phone) + "' and id ='" + std::string(id) + "'";;

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateQUEUE(const char *id, const char *phone, const char *time) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE -> Data (updateQUEUE) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// получение последнего ID актуального
int SQL_REQUEST::SQL::getLastIDQUEUE(const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getLastIDQUEUE");
		return -1;
	}

	const std::string query = "select id from queue where phone = "
		+ std::string(phone) + " and date_time > '"
		+ getCurrentStartDay() + "' order by date_time desc limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::getLastIDQUEUE(const char *phone) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что нет записи		
		showErrorBD("SQL_REQUEST::SQL::getLastIDQUEUE -> query("+query+")",&this->mysql);
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	int count = std::stoi(row[0]);
	
	mysql_free_result(result);	

	return count;
}

// обновление данных таблицы QUEUE о том с кем сейчас разговаривает оператор
void SQL_REQUEST::SQL::updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_SIP");
		return;
	}

	// проверим есть ли такой номер 	
	if (isExistQUEUE_SIP(phone))
	{ // номер существует, обновляем данные
		std::string id = std::to_string(getLastIDQUEUE(phone));

		std::string query = "update queue set sip = '" + std::string(sip) + "', talk_time = '"+ getTalkTime(talk_time) + "', answered ='1' where phone = '" + std::string(phone) + "' and id ='" + std::string(id) + "'";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time) -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD("SQL_REQUEST::SQL::updateQUEUE_SIP -> Data (updateQUEUE_SIP) error -> query("+query+")", &this->mysql);
		};

		mysql_close(&this->mysql);		
	}	
}

// существует ли такой номер в таблице QUEUE чтобы добавить sip оператора который с разговор ведет
bool SQL_REQUEST::SQL::isExistQUEUE_SIP(const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE_SIP");
		return true;
	}
	
	const std::string query = "select count(phone) from queue where phone = '" + std::string(phone)
							+ "' and date_time > '" + getCurrentStartDay() 
							+ "' order by date_time desc limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistQUEUE_SIP(const char *phone) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE_SIP -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existQueueSip;
	(std::stoi(row[0]) == 0 ? existQueueSip = false : existQueueSip = true);
	
	mysql_free_result(result);

	return existQueueSip;
	
}

//обновление данных когда звонок не дождался своей очереди
void SQL_REQUEST::SQL::updateQUEUE_fail(const std::vector<QUEUE::Pacients> &pacient_list)
{
	
	// найдем текущие номера которые мы не будет трогать при обновлении
	std::string list_phone;
	
	for (const auto &list : pacient_list) {
		
		if (list_phone.empty()) {
			list_phone = "'"+list.phone+"'";
		}
		else {
			list_phone = list_phone + ",'" + list.phone + "'";
		}		
	}	

	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_fail");
		return;
	}

	// обновляем данные
	std::string query = "update queue set fail = '1' where date_time > '" + getCurrentStartDay() + "' and answered = '0'"
		+ " and sip = '-1' and phone not in ("+ list_phone +")";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateQUEUE_fail(const std::vector<QUEUE::Pacients> &pacient_list) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_fail -> Data (updateQUEUE_fail) error -> query(" + query + ")", &this->mysql);
	};

	mysql_close(&this->mysql);
}

// обновление данных когда звонок не дождался своей очереди
void SQL_REQUEST::SQL::updateQUEUE_fail()
{
	// обновляем данные
	std::string query = "update queue set fail = '1' where date_time > '" + getCurrentDateTimeAfter20hours() + "' and answered = '0' and sip = '-1' ";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateQUEUE_fail() -> " + query);
		}
	}

	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_fail");
		return;
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_fail -> Data (updateQUEUE_fail) error -> query(" + query + ")", &this->mysql);
	};

	mysql_close(&this->mysql);
}

// обновление данных когда у нас звонок из IVR попал в очередь
void SQL_REQUEST::SQL::updateIVR_to_queue(const std::vector<QUEUE::Pacients> &pacient_list)
{
	// найдем текущие номера которые будем трогать при обновлении
	std::string list_phone;

	for (const auto &list : pacient_list)
	{
		if (list_phone.empty())
		{
			list_phone = "'" + list.phone + "'";
		}
		else
		{
			list_phone = list_phone + ",'" + list.phone + "'";
		}
	}

	// обновляем данные
	std::string query = "update ivr set to_queue = '1' where date_time > '" + getCurrentDateTimeAfterMinutes(5)+"' and phone in(" + list_phone + ") and to_queue = '0'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateIVR_to_queue(const std::vector<QUEUE::Pacients> &pacient_list) -> " + query);
		}
	}

	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateIVR_to_queue");
		return;
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateIVR_to_queue -> Data (updateIVR_to_queue) error -> query(" + query + ")", &this->mysql);
	};

	mysql_close(&this->mysql);
}


// проверка есть ли номера которые позвонили после 20:00
bool SQL_REQUEST::SQL::isExistQueueAfter20hours()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistQueueAfter20hours");
		return true;
	}

	const std::string query = "select count(phone) from queue where date_time > '"
		+ getCurrentDateTimeAfter20hours() + "' and sip = '-1' and answered = '0' and fail = '0' order by date_time desc ";
	
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistQueueAfter20hours() -> " + query);
		}
	}
	
	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQueueAfter20hours -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existQueueAfter20hours;
	(std::stoi(row[0]) == 0 ? existQueueAfter20hours = false : existQueueAfter20hours = true);

	mysql_free_result(result);	
	mysql_close(&this->mysql); 
			
	return existQueueAfter20hours;
}

// обновление поля hash когда успешно поговорили
void SQL_REQUEST::SQL::updateQUEUE_hash(const std::vector<QUEUE::Pacients> &pacient_list)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_hash");
		return;
	}	

	// найдем текущие номера которые будем трогать при обновлении
	std::string list_phone;

	for (const auto &list : pacient_list)
	{
		if (list_phone.empty())
		{
			list_phone = "'" + list.phone + "'";
		}
		else
		{
			list_phone = list_phone + ",'" + list.phone + "'";
		}
	} 

	QUEUE::QueueBD queuebd;
	
	
	const std::string query = "select id,phone,date_time from queue where date_time > '"
		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL and phone not in("+ list_phone+")";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateQUEUE_hash(const std::vector<QUEUE::Pacients> &pacient_list) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::updateQUEUE_hash -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row; 
	
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		QUEUE::BD bd;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			
			if (i == 0) {
				bd.id = row[i];
			}
			else if (i == 1) {
				bd.phone = row[i];
			}
			else if (i == 2) {
				bd.date_time = row[i];
			}			
		}

		bd.hash = std::hash<std::string>()(bd.phone+"_"+bd.date_time);
		queuebd.list.push_back(bd);		
	}	
	
	mysql_free_result(result);
	
	// обновляем
	for (const auto &list : queuebd.list)	{

		std::string query = "update queue set hash = '" + std::to_string(list.hash)
			+ "' where id ='" + list.id
			+ "' and phone ='" + list.phone
			+ "' and date_time = '" + list.date_time + "'";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::updateQUEUE_hash(const std::vector<QUEUE::Pacients> &pacient_list) -> " + query);
			}
		}

		if (!isConnectedBD())
		{
			showErrorBD("SQL_REQUEST::SQL::updateQUEUE_hash");
			return;
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD("SQL_REQUEST::SQL::updateQUEUE_hash -> Data (updateQUEUE_hash) error -> query(" + query + ")", &this->mysql);
		};

	};
	mysql_close(&this->mysql); 
}


// проверка есть ли номера которым нужно проставить статус отвечено после того как оператор ушел из линии
bool SQL_REQUEST::SQL::isExistAnsweredAfter20hours()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistAnsweredAfter20hours");
		return true;
	}

	const std::string query = "select count(id) from queue where date_time > '"
		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";


	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistAnsweredAfter20hours() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistAnsweredAfter20hours -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existAnsweredAfter20;
	(std::stoi(row[0]) == 0 ? existAnsweredAfter20 = false : existAnsweredAfter20 = true);

	mysql_free_result(result);
	mysql_close(&this->mysql); 

	return existAnsweredAfter20;
}

// обновление данных когда оператор поговорил и ушел из линии, а звонок все еще находится не обработанным
void SQL_REQUEST::SQL::updateAnswered_fail()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateAnswered_fail");
		return;
	}	

	QUEUE::QueueBD queuebd;

	const std::string query = "select id,phone,date_time from queue where date_time > '"
		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateAnswered_fail() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::updateAnswered_fail -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		QUEUE::BD bd;
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{

			if (i == 0)
			{
				bd.id = row[i];
			}
			else if (i == 1)
			{
				bd.phone = row[i];
			}
			else if (i == 2)
			{
				bd.date_time = row[i];
			}
		}

		bd.hash = std::hash<std::string>()(bd.phone + "_" + bd.date_time);
		queuebd.list.push_back(bd);
	}

	mysql_free_result(result);

	// обновляем
	for (const auto &list : queuebd.list)
	{

		std::string query = "update queue set hash = '" + std::to_string(list.hash)
			+ "' where id ='" + list.id
			+ "' and phone ='" + list.phone
			+ "' and date_time = '" + list.date_time + "'";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add("SQL_REQUEST::SQL::updateAnswered_fail() -> " + query);
			}
		}

		if (!isConnectedBD())
		{
			showErrorBD("SQL_REQUEST::SQL::updateAnswered_fail");
			return;
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD("SQL_REQUEST::SQL::updateAnswered_fail -> Data (updateAnswered_fail) error -> query(" + query + ")", &this->mysql);
		};

	};
	mysql_close(&this->mysql);
}


// добавление sip номере оператора + его очередь
void SQL_REQUEST::SQL::insertOperatorsQueue(const char *sip, const char *queue)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertOperatorsQueue");
		return;
	}
	
	std::string query = "insert into operators_queue (sip,queue) values ('"
		+ std::string(sip) + "','"
		+ std::string(queue) + "')";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::insertOperatorsQueue(const char *sip, const char *queue) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertOperatorsQueue -> Data (insertOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}
	

	mysql_close(&this->mysql);
}

// очистка таблицы operators_queue
void SQL_REQUEST::SQL::clearOperatorsQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::clearOperatorsQueue");
		return;
	}
	
	std::string query = "delete from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::clearOperatorsQueue() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::clearOperatorsQueue -> Data (insertOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}	

	mysql_close(&this->mysql);
}

// проверка существует ли такой sip+очередь в БД
bool SQL_REQUEST::SQL::isExistOperatorsQueue(const char *sip, const char *queue)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue");
		return true;
	}

	const std::string query = "select count(id) from operators_queue where sip = '"+ std::string(sip) + "' and queue = '"+std::string(queue)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistOperatorsQueue(const char *sip, const char *queue) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);	

	bool existOperatos;
	std::stoi(row[0]) == 0 ? existOperatos = false : existOperatos = true;
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return existOperatos;

}

// проверка существует ли хоть 1 запись в БД sip+очередь
bool SQL_REQUEST::SQL::isExistOperatorsQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue");
		return true;
	}

	const std::string query = "select count(id) from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistOperatorsQueue() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existOperatorsQueue;
	(std::stoi(row[0]) == 0 ? existOperatorsQueue = false : existOperatorsQueue = true);

	mysql_free_result(result);
	mysql_close(&this->mysql);

	return existOperatorsQueue;		
}

// удаление sip номера оператора из всех очередей
void SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue()");
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip) -> " + query);
		}
	}
	

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue -> Data (deleteOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// удаление sip номера оператора и конкретной очереди
void SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip, const std::string &queue)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue(2 param)");
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "' and queue = '" + queue + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip, const std::string &queue) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue(2 param) -> Data (deleteOperatorsQueue(2 param)) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);

}


// проверка текущих sip + очередь
void SQL_REQUEST::SQL::checkOperatorsQueue(const std::vector<ACTIVE_SIP::Operators> &list_operators)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::checkOperatorsQueue()");
		return;
	}

	// найдем данные по БД
	ACTIVE_SIP::Operators curr_list_operator;
	std::vector<ACTIVE_SIP::Operators> list_operators_bd;

	const std::string query = "select sip,queue from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::checkOperatorsQueue(const std::vector<ACTIVE_SIP::Operators> &list_operators) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::checkOperatorsQueue -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL) {		
		
		curr_list_operator.clear();

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{			
			
			if (i == 0) {
				curr_list_operator.sip_number = row[i];
			}
			else if (i == 1) {
				curr_list_operator.queue.push_back(row[i]);
			}					
							
		}	

		list_operators_bd.push_back(curr_list_operator);		
	}

	mysql_free_result(result);
	mysql_close(&this->mysql);

	// проверим совпадают ли данные с данными по БД
	for (const auto &curr_list : list_operators_bd)	{
		bool isExistSip{ true };	// считаем что sip по умолчанию существует
		bool isExistQueue{ true };	// считаем что queue по умолчанию существует

		for (const auto &memory_list : list_operators) {
			if (curr_list.sip_number == memory_list.sip_number) {
				isExistSip = true;
				
				// проверим есть ли такая очередь
				for (unsigned int i = 0; i < memory_list.queue.size(); ++i) {
					if (curr_list.queue[0] == memory_list.queue[i]) {
						isExistQueue = true;
						break;
					}
					else {
						isExistQueue = false;						
					}
				}
				
				break;

			}
			else {
				// нет sip, надо удалить из БД
				isExistSip = false;
			}
		}
		
		// что именно будем удалять из БД
		if (isExistSip) {
			if (!isExistQueue) {
				// удаляем sip + очередь конкретную
				SQL_REQUEST::SQL base;
				
				if (base.isConnectedBD())
				{
					base.deleteOperatorsQueue(curr_list.sip_number, curr_list.queue[0]);
				}
			
			}
		}
		else {
			// удаляем весь sip
			SQL_REQUEST::SQL base;

			if (base.isConnectedBD())
			{
				base.deleteOperatorsQueue(curr_list.sip_number);
			}
		}
	}		
}




// сколько всего позвонило на линию IVR
int SQL_REQUEST::SQL::getIVR_totalCalls()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getIVR_totalCalls()");
		return 0;
	}

	const std::string query = "select count(phone) from ivr where date_time > '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::getIVR_totalCalls() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::getIVR_totalCalls -> query(" + query + ")", &this->mysql);
		return 0;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return std::stoi(row[0]);
}

// сколько всего позвонило на линию IVR (поиск по trunk)
int SQL_REQUEST::SQL::getIVR_totalCalls(const IVR::CallerID &trunk)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getIVR_totalCalls(1param)");
		return 0;
	}

	const std::string query = "select count(phone) from ivr where trunk ='" + IVR::getCallerID(trunk) + "'  and date_time > '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::getIVR_totalCalls(const IVR::CallerID &trunk) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::getIVR_totalCalls(1param) -> query(" + query + ")", &this->mysql);
		return 0;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); // под вопросом?

	return std::stoi(row[0]);;
}

// сколько всего ответило и сколько пропущенных
int SQL_REQUEST::SQL::getQUEUE_Calls(bool answered)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getQUEUE_Calls(1 param)");
		return 0;
	}
	
{
	std::string query;
		switch (answered)
	{
		case(true):
		{
			query = "select count(phone) from queue where date_time > '" + getCurrentStartDay() + "' and answered = '1' and hash is not NULL";
			break;
		}
		case(false):
		{
			query = "select count(phone) from queue where date_time > '" + getCurrentStartDay() + "' and fail = '1'";
			break;
		}
	}
	
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::getQUEUE_Calls(bool answered) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::getQUEUE_Calls(1param) -> query(" + query + ")", &this->mysql);
		return 0;
	}
}
	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); // под вопросом?

	return std::stoi(row[0]);
}

// сколько всего было в очереди
int SQL_REQUEST::SQL::getQUEUE_Calls()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::getQUEUE_Calls");
		return 0;
	}		
	 const std::string query = "select count(phone) from queue where date_time > '" + getCurrentStartDay()+"'";

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::getQUEUE_Calls -> query(" + query + ")", &this->mysql);
		return 0;
	}
	
	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); // под вопросом?

	return std::stoi(row[0]);
}

// проверка если новая команда для  входа\выхода из очереди
bool SQL_REQUEST::SQL::remoteCheckNewCommads()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::remoteCheckNewCommads");
		return true;
	}

	const std::string query = "select count(id) from remote_commands";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::remoteCheckNewCommads() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::remoteCheckNewCommads -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);			

	bool existNewCommand;
	((std::stoi(row[0]) == 0) ? existNewCommand = false : existNewCommand = true);
	
	mysql_free_result(result);
	mysql_close(&this->mysql);
	
	return existNewCommand;
}


// генерация текущиъ найденных команд
void SQL_REQUEST::SQL::createListRemoteCommands(std::vector<REMOTE_COMMANDS::R_Commands> &list)
{	
	
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::createListRemoteCommands");
		return;
	}	
	// формируем лист 
	const std::string query = "select id,sip,command,ip,user_id,user_login_pc,pc from remote_commands";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::createListRemoteCommands(std::vector<REMOTE_COMMANDS::R_Commands> &list) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::createListRemoteCommands -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)	{
		
		
		int id;						// id команды (для удобного поиска в запросе)
		std::string sip;			// sip инициализировавший команду
		LOG::Log command;			// сама команда (int)
		std::string ip;				// ip с которого пришла команда
		int user_id;				// id пользователя по БД
		std::string user_login_pc;	// логин зареган на пк с которого пришла команда
		std::string pc;				// имя пк с которого отправили коиманду


		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{

			if (i == 0)	{
				id = std::stoi(row[i]);
			}
			else if (i == 1) {
				sip = row[i];
			}
			else if (i == 2) {
				  command = getRemoteCommand(std::stoi(row[i]));
			}
			else if (i == 3) {
				ip = row[i];
			}
			else if (i == 4) {
				user_id = std::stoi(row[i]);
			}
			else if (i == 5) {
				user_login_pc = row[i];
			}
			else if (i == 6) {
				pc = row[i];
			}
		}

		// добавим нужную команду в список всех найдекнных команд
		list.push_back({id,sip,command,ip,user_id,user_login_pc,pc});
	}


	mysql_free_result(result);	
	mysql_close(&this->mysql);
	
}

// запуск удаленной команды
void SQL_REQUEST::SQL::startRemoteCommand(int id, std::string sip, LOG::Log command, int user_id)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::startRemoteCommand");
		return;
	}

	// запускаем команду
	std::string responce_now;	
	
	// в какую очередь поставить
	CONSTANTS::AsteriskQueue curr_queue;


	switch (command)
	{
		// добавленние в очередь 5000 или 5050	
		case LOG::Log::Log_add_queue_5000 ... LOG::Log::Log_add_queue_5050 :  
		{			
			std::string responce = CONSTANTS::cRemoteCommandResponseAdd;
			
			// заменяем % на нужное нам
			std::string repl_sip = "%sip";			
			 
			size_t position = responce.find(repl_sip);

			while (position != std::string::npos) {
				responce.replace(position, repl_sip.length(), sip);
				position = responce.find(repl_sip);
			}
			
			std::string repl_queue = "%queue";
			position = responce.find(repl_queue);
			
			//в какую очередь поставить оператора
			if (command == LOG::Log::Log_add_queue_5000) {
				curr_queue = CONSTANTS::AsteriskQueue::main;
			}
			else if (command == LOG::Log::Log_add_queue_5050) {
				curr_queue = CONSTANTS::AsteriskQueue::lukoil;
			}

			responce.replace(position, repl_queue.length(), getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(curr_queue)));
			responce_now = responce;			

			if (!CONSTANTS::DEBUG_MODE)
			{
				system(responce_now.c_str());
			}

			break;
		}		
		// добавление в очередь 5000+5050
		case LOG::Log::Log_add_queue_5000_5050: {		 
			
			LOG::Log command = LOG::Log::Log_add_queue_5000;			
			// ставим 5000
			startRemoteCommand(id,sip, command, user_id);


			command = LOG::Log::Log_add_queue_5050;
			// ставим 5050
			startRemoteCommand(id, sip, command, user_id);
			
			break;
		}	
		case LOG::Log::Log_del_queue_5000 ... LOG::Log::Log_del_queue_5050:
			{

			std::string responce = CONSTANTS::cRemoteCommandResponseDel;

			// заменяем % на нужное нам
			std::string repl_sip = "%sip";

			size_t position = responce.find(repl_sip);
			
			responce.replace(position, repl_sip.length(), sip);			

			std::string repl_queue = "%queue";
			position = responce.find(repl_queue);

			//в какую очередь поставить оператора
			if (command == LOG::Log::Log_del_queue_5000)
			{
				curr_queue = CONSTANTS::AsteriskQueue::main;
			}
			else if (command == LOG::Log::Log_del_queue_5050)
			{
				curr_queue = CONSTANTS::AsteriskQueue::lukoil;
			}

			responce.replace(position, repl_queue.length(), getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(curr_queue)));
			responce_now = responce;

			if (!CONSTANTS::DEBUG_MODE)
			{
				system(responce_now.c_str());
			}

			break;
		}	
			// удаление из очереди 5000+5050
		case LOG::Log::Log_del_queue_5000_5050:
		{

			LOG::Log command = LOG::Log::Log_del_queue_5000;
			// ставим 5000
			startRemoteCommand(id, sip, command, user_id);


			command = LOG::Log::Log_del_queue_5050;
			// ставим 5050
			startRemoteCommand(id, sip, command, user_id);

			break;
		}
			// исход
		case LOG::Log::Log_home ... LOG::Log::Log_reserve : {
			// ну тут все просто не зависимо от того в какой очереди находимся выходим из всех очередей
			LOG::Log command = LOG::Log::Log_del_queue_5000_5050;			
			startRemoteCommand(id, sip, command, user_id);
		}
	}
	
}

// удаление успешно выполненной команды
void SQL_REQUEST::SQL::deleteRemoteCommand(int id)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteRemoteCommand");
		return;
	}

	std::string query = "delete from remote_commands where id = '"+std::to_string(id)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteRemoteCommand(int id) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteRemoteCommand -> Data (deleteRemoteCommand) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// обновление текущего статуса оператора
void SQL_REQUEST::SQL::updateStatusOperators(int user_id, REMOTE_COMMANDS::StatusOperators status)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateStatusOperators");
		return;
	}
	
	std::string query = "update operators set status = '" + std::to_string(getStatusOperators(status)) + "' where user_id = '" + std::to_string(user_id) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::updateStatusOperators(int user_id, REMOTE_COMMANDS::StatusOperators status) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::updateStatusOperators -> Data (updateStatusOperators) error -> query(" + query + ")", &this->mysql);
	};

	
	mysql_close(&this->mysql);
	
}

// создание лога в БД
void SQL_REQUEST::SQL::addLog(LOG::Log command, int base_id)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::addLog");
		return;
	}


	// найдем все данные по пользователю для логирования
	const std::string query = "select sip,ip,user_id,user_login_pc,pc from remote_commands where id = '"+ std::to_string(base_id)+"' limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::addLog(LOG::Log command, int base_id) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::addLog -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;
	
	std::string sip;			// sip инициализировавший команду	
	std::string ip;				// ip с которого пришла команда
	int user_id;				// id пользователя по БД
	std::string user_login_pc;	// логин зареган на пк с которого пришла команда
	std::string pc;				// имя пк с которого отправили коиманду

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)
			{
				sip = row[i];
			}
			else if (i == 1)
			{
				ip = row[i];
			}
			else if (i == 2)
			{
				user_id = std::stoi(row[i]);
			}
			else if (i == 3)
			{
				user_login_pc = row[i];
			}
			else if (i == 4)
			{
				pc = row[i];
			}			
		}		
	}

	mysql_free_result(result);	
	
	// устанавливаем данные в лог
	std::string query_insert = "insert into logging (ip,user_id,user_login_pc,pc,action) values ('" + ip + 
																								"','" + std::to_string(user_id) + 
																								"','" + user_login_pc + 
																								"','" + pc + 
																								"','" + std::to_string(getRemoteCommand(command)) + "')";
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::addLog(LOG::Log command, int base_id) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertIVR -> Data (insertIVR) error -> query(" + query_insert + ")", &this->mysql);
	}
	
	mysql_close(&this->mysql);
}


void SQL_REQUEST::SQL::execTaskQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::execTaskQueue");
		return;
	}

	// найдем все данные 
	const std::string query = "select * from queue where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::execTaskQueue() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::execTaskQueue -> query(" + query + ")", &this->mysql);
		return;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::Queue> listQueue;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::Queue queue;
		
		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				queue.fileds.id = std::atoi(row[i]);
			}
			else if (i == 1)	// number_queue
			{
				queue.number_queue = std::atoi(row[i]);
			}
			else if (i == 2)   // phone
			{
				queue.fileds.phone = row[i];
			}
			else if (i == 3) // waiting_time
			{
				queue.fileds.waiting_time = row[i];
			}
			else if (i == 4) // date_time
			{
				queue.fileds.date_time = row[i];
			}
			else if (i == 5) // sip
			{
				queue.sip = std::atoi(row[i]);
			}
			else if (i == 6) { // talk_time (может быть NULL!)
				if (row[i]) {
					queue.talk_time = row[i];
				}				
			} 
			else if (i == 7) { //answered
				queue.answered = std::atoi(row[i]);
			} 
			else if (i == 8) { // fail
				queue.fail = std::atoi(row[i]);
			}
			else if (i == 9) { // hash (может быть NULL!)
				if (row[i]) {
					queue.hash = string_to_size_t(row[i]);
				}			
			}
		}

		listQueue.emplace_back(queue);
	}

	if (!listQueue.empty()) { 
		std::cout << "HouseKeeping.Queue work...\n";

		// перекидывание 1 транзакции хза раз ? \ или по 100 надо подумать..
		SQL_REQUEST::SQL base;

		for (auto &list : listQueue)
		{		
			if (base.insertDataTaskQueue(list)) {
				// удаляем текущий добавленный
				base.deleteDataTaskQueue(list.fileds.id);
			}	
		}
		
		std::cout << "HouseKeeping.Queue work DONE!\n";

		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
	else {
		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
}

void SQL_REQUEST::SQL::execTaskLogging()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::execTaskLogging");
		return;
	}

	// найдем все данные 
	const std::string query = "select * from logging where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::execTaskLogging() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::execTaskLogging -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::Logging> listLogging;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::Logging logging;

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				logging.fileds.id = std::atoi(row[i]);
			}
			else if (i == 1)	// ip
			{
				logging.ip = row[i];
			}
			else if (i == 2)   // user_id
			{
				logging.user_id = std::atoi(row[i]);
			}
			else if (i == 3) // user_login_pc
			{
				logging.user_login_pc = row[i];
			}
			else if (i == 4) // pc
			{
				logging.pc = row[i];
			}
			else if (i == 5) // date_time
			{
				logging.fileds.date_time = row[i];
			}
			else if (i == 6)
			{ 		
				logging.action = std::atoi(row[i]);				
			}			
		}

		listLogging.emplace_back(logging);
	}

	if (!listLogging.empty())
	{
		std::cout << "HouseKeeping.Logging work...\n";
		// перекидывание 1 транзакции хза раз ? \ или по 100 надо подумать..
		SQL_REQUEST::SQL base;

		for (auto &list : listLogging)
		{
			if (base.insertDataTaskLogging(list))
			{
				// удаляем текущий добавленный
				base.deleteDataTaskLogging(list.fileds.id);
			}
		}
		std::cout << "HouseKeeping.Logging work DONE!\n";

		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
	else {
		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
}

void SQL_REQUEST::SQL::execTaskIvr()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::execTaskIvr");
		return;
	}

	// найдем все данные 
	const std::string query = "select * from ivr where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::execTaskIvr() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::execTaskIvr -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::IVR> listIvr;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::IVR ivr;

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				ivr.fileds.id = std::atoi(row[i]);
			}
			else if (i == 1)	// phone
			{
				ivr.fileds.phone = row[i];
			}
			else if (i == 2)   // waiting_time
			{
				ivr.fileds.waiting_time = row[i];
			}
			else if (i == 3) // date_time
			{
				ivr.fileds.date_time = row[i];
			}
			else if (i == 4) // trunk
			{
				ivr.trunk = row[i];
			}
			else if (i == 5) // to_queue
			{
				ivr.to_queue = std::atoi(row[i]);
			}
			else if (i == 6)
			{
				ivr.to_robot = std::atoi(row[i]);
			}
		}

		listIvr.emplace_back(ivr);
	}


	if (!listIvr.empty())
	{
		std::cout << "HouseKeeping.Ivr work...\n";

		// перекидывание 1 транзакции хза раз ? \ или по 100 надо подумать..
		SQL_REQUEST::SQL base;

		for (auto &list : listIvr)
		{
			if (base.insertDataTaskIvr(list))
			{
				// удаляем текущий добавленный
				base.deleteDataTaskIvr(list.fileds.id);
			}
		}

		std::cout << "HouseKeeping.Ivr work DONE!\n";

		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
	else {
		mysql_free_result(result);
		mysql_close(&this->mysql);
	}

}

void SQL_REQUEST::SQL::execTaskOnHold()
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::execTaskOnHold");
		return;
	}

	// найдем все данные 
	const std::string query = "select * from operators_ohhold where date_time_start < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::execTaskOnHold() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::execTaskOnHold -> query(" + query + ")", &this->mysql);
		return;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::OnHold> listOnHold;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::OnHold onHold;

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				onHold.id = std::atoi(row[i]);				
			}
			else if (i == 1)	// sip
			{
				onHold.sip = std::atoi(row[i]);
			}
			else if (i == 2)   // date_time_start
			{
				onHold.date_time_start = row[i];
			}
			else if (i == 3) // date_time_stop
			{
				onHold.date_time_stop = row[i];
			}
			else if (i == 4) // hash
			{
				onHold.hash = string_to_size_t(row[i]);
			}
		}

		listOnHold.emplace_back(onHold);
	}

	if (!listOnHold.empty())
	{
		std::cout << "HouseKeeping.OnHold work...\n";
		// перекидывание 1 транзакции хза раз ? \ или по 100 надо подумать..
		SQL_REQUEST::SQL base;

		for (auto &list : listOnHold)
		{
			if (base.insertDataTaskOnHold(list))
			{
				// удаляем текущий добавленный
				base.deleteDataTaskOnHold(list.id);
			}
		}
		std::cout << "HouseKeeping.OnHold work DONE!\n";

		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
	else
	{
		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
}


bool SQL_REQUEST::SQL::insertDataTaskQueue(HOUSEKEEPING::Queue &queue)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskQueue");
		return false;
	}

	std::string query_insert;
	
	if (queue.sip != -1) {	 // на случай если нет NULL полей

		// устанавливаем данные в history_queue
	 query_insert = "insert into history_queue (id,number_queue,phone,waiting_time,date_time,sip,talk_time,answered,fail,hash) values ('" + std::to_string(queue.fileds.id) +
			"','" + std::to_string(queue.number_queue) +
			"','" + queue.fileds.phone +
			"','" + queue.fileds.waiting_time +
			"','" + queue.fileds.date_time +
			"','" + std::to_string(queue.sip) +
			"','" + queue.talk_time +
			"','" + std::to_string(queue.answered) +
			"','" + std::to_string(queue.fail) +
			"','" + std::to_string(queue.hash) + "')";
	}
	else {					 // на случай если есть NULL поля

	 query_insert = "insert into history_queue (id,number_queue,phone,waiting_time,date_time,sip,answered,fail) values ('" + std::to_string(queue.fileds.id) +
			"','" + std::to_string(queue.number_queue) +
			"','" + queue.fileds.phone +
			"','" + queue.fileds.waiting_time +
			"','" + queue.fileds.date_time +
			"','" + std::to_string(queue.sip) +
			"','" + std::to_string(queue.answered) +
			"','" + std::to_string(queue.fail) + "')";
	}

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::insertDataTaskQueue(HOUSEKEEPING::Queue &queue) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskQueue -> Data (insertDataTaskQueue) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);
	
	return true;
}


bool SQL_REQUEST::SQL::deleteDataTaskQueue(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskQueue");
		return false;
	}

	std::string query = "delete from queue where id = '"+std::to_string(ID)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteDataTaskQueue(int ID) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskQueue -> Data (deleteDataTaskQueue) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskLogging(HOUSEKEEPING::Logging &logging)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskLogging");
		return false;
	}	

   // устанавливаем данные в history_logging
	std::string	query_insert = "insert into history_logging (id,ip,user_id,user_login_pc,pc,date_time,action) values ('" + std::to_string(logging.fileds.id) +
			"','" + logging.ip +
			"','" + std::to_string(logging.user_id) +
			"','" + logging.user_login_pc +
			"','" + logging.pc +
			"','" + logging.fileds.date_time +
			"','" + std::to_string(logging.action) + "')";
	
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::insertDataTaskLogging(HOUSEKEEPING::Logging &logging) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskLogging -> Data (insertDataTaskLogging) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::deleteDataTaskLogging(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskLogging");
		return false;
	}

	std::string query = "delete from logging where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteDataTaskLogging(int ID) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskLogging -> Data (deleteDataTaskLogging) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskIvr(HOUSEKEEPING::IVR &ivr)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskIvr");
		return false;
	}

	// устанавливаем данные в history_ivr
	std::string	query_insert = "insert into history_ivr (id,phone,waiting_time,date_time,trunk,to_queue,to_robot) values ('" + std::to_string(ivr.fileds.id) +
		"','" + ivr.fileds.phone +
		"','" + ivr.fileds.waiting_time +
		"','" + ivr.fileds.date_time +
		"','" + ivr.trunk +
		"','" + std::to_string(ivr.to_queue) +
		"','" + std::to_string(ivr.to_robot) + "')";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::insertDataTaskIvr(HOUSEKEEPING::IVR &ivr) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskIvr -> Data (insertDataTaskIvr) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::deleteDataTaskIvr(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskIvr");
		return false;
	}

	std::string query = "delete from ivr where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteDataTaskIvr(int ID) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskIvr -> Data (deleteDataTaskIvr) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskOnHold(HOUSEKEEPING::OnHold &onHold)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskOnHold");
		return false;
	}

	// устанавливаем данные в history_ivr
	std::string	query_insert = "insert into history_onhold (id,sip,date_time_start,date_time_stop,hash) values ('" + std::to_string(onHold.id) +
		"','" + std::to_string(onHold.sip) +
		"','" + onHold.date_time_start +
		"','" + onHold.date_time_stop +
		"','" + std::to_string(onHold.hash) +"')";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::insertDataTaskOnHold(HOUSEKEEPING::OnHold &onHold) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskOnHold -> Data (insertDataTaskOnHold) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::deleteDataTaskOnHold(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskOnHold");
		return false;
	}

	std::string query = "delete from operators_ohhold where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::deleteDataTaskOnHold(int ID) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteDataTaskOnHold -> Data (deleteDataTaskOnHold) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

void SQL_REQUEST::SQL::updateOperatorsOnHold(ACTIVE_SIP::Parsing *list)
{
	typedef std::vector<ACTIVE_SIP::Operators>	operators;
	typedef std::vector<ACTIVE_SIP::OnHold>		operators_onhold;
	
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::updateOperatorsOnHold");
		return;
	}

	// найдем все sip операторы которые числяться по БД в статусе onHold	
	auto onHold = createOnHoldSip();
	
	
	

		

	if (onHold->size() >= 3) {
		std::cout << "test";
	}

	// проверяем
	if (!onHold->empty()) {
		// проверяем есть ли сейчас операторы с onHold
		operators curr_list_operators = list->getListOperators();
		
		// переменная на случай когда надо убрать из onHold значения, т.к. отключили onHold, а в памяти он еще остался
		bool needCheckOnHold{ false };

		// проверяем сначало текущие которые уже были в onHold добавлены		
		if (!curr_list_operators.empty())
		{				
			bool isExistOnHold{ true };

			// проверим оператор еще разговаривает в onHold или уже нет
			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
			{
				bool isExistOnHold{ true };
				
				if (needCheckOnHold) 
				{
					break;
				}

				for (operators::iterator list_operators = curr_list_operators.begin(); list_operators != curr_list_operators.end(); ++list_operators)
				{
					if (operators_hold->sip_number == list_operators->sip_number)
					{
						list_operators->isOnHold ? isExistOnHold = true : isExistOnHold = false;
						
						// оператор перестал быть в onHold ставим время что он закончил этот статус
						if (!isExistOnHold)
						{
							SQL base;
							base.disableOperatorsOnHold(*operators_hold);
							
							// занесем в базу что нужно убрать из onHold
							needCheckOnHold = true;							
						} 				
					}					
				}				
			}

			// вдруг новые onHold появились, добавляем в БД, но сначало проверим delOnHold
			if (needCheckOnHold) {
				auto onHoldNeedCheck = createOnHoldSip();

				if (!isExistNewOnHoldOperators(onHoldNeedCheck, curr_list_operators))
				{					
					auto new_list = createNewOnHoldOperators(*onHoldNeedCheck, curr_list_operators);

					for (auto iter = new_list->begin(); iter != new_list->end(); ++iter)
					{
						SQL base;
						base.addOperatorsOnHold(iter->first, iter->second);
					}

					delete new_list;
				}

				delete onHoldNeedCheck;
			}
			else {
				if (!isExistNewOnHoldOperators(onHold, curr_list_operators))
				{				
					auto new_list = createNewOnHoldOperators(*onHold, curr_list_operators);

					for (auto iter = new_list->begin(); iter != new_list->end(); ++iter)
					{
						SQL base;
						base.addOperatorsOnHold(iter->first,iter->second);
					}

					delete new_list;
				}
			}
		}
		else
		{ // очищаем список т.к. операторов нет аквтиных
			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
			{
				SQL base;
				base.disableOperatorsOnHold(*operators_hold);
			}
		}				
		
	}
	else {	// пустой проверяем что у нас сейчас на данный момент есть и добавляем в БД
		operators curr_list_operators = list->getListOperators();
		
		if (!curr_list_operators.empty()) {
			for (operators::iterator it = curr_list_operators.begin(); it != curr_list_operators.end(); ++it) {
				if (it->isOnHold) {

					SQL base;
					base.addOperatorsOnHold(it->sip_number,it->phoneOnHold);

				}
			}
		}		
	}

	delete onHold;
}

std::vector<ACTIVE_SIP::OnHold> *SQL_REQUEST::SQL::createOnHoldSip()
{
	auto *listHold = new std::vector<ACTIVE_SIP::OnHold>;

	
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::createOnHoldSip");
		return listHold;
	}

	// найдем все данные 
	//const std::string query = "select * from operators_ohhold where date_time_start > '" + getCurrentStartDay() + "' and date_time_stop is NULL";
	const std::string query = "select operators_ohhold.id, operators_ohhold.sip, operators_ohhold.date_time_start, operators_ohhold.date_time_stop, operators_ohhold.hash, queue.phone from queue inner join operators_ohhold on queue.sip = operators_ohhold.sip where queue.hash is NULL and operators_ohhold.date_time_stop is NULL";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("*SQL_REQUEST::SQL::createOnHoldSip() -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка		
		showErrorBD("SQL_REQUEST::SQL::createOnHoldSip -> query(" + query + ")", &this->mysql);
		return listHold;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;	

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		ACTIVE_SIP::OnHold hold;

		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				hold.id = std::atoi(row[i]);
			}
			else if (i == 1)	// sip
			{
				hold.sip_number = row[i];
			}
			else if (i == 2)   // date_time_start
			{
				hold.date_time_start = row[i];
				
			}
			else if (i == 3) // date_time_stop
			{
				if (row[i])	{
					hold.date_time_stop = row[i];
				}
			}
			else if (i == 4) // hash
			{
				hold.hash = string_to_size_t(row[i]);
			}
			else if (i == 5) // phone
			{
				hold.phone = row[i];
			}
		}

		listHold->push_back(hold);
	}
	
	mysql_free_result(result);
	mysql_close(&this->mysql);
	return listHold;
}

void SQL_REQUEST::SQL::addOperatorsOnHold(const std::string &sip, const std::string &phone)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::addOperatorsOnHold");
		return;
	}
	
	std::string curr_date = getCurrentDateTime();
	size_t hash = std::hash<std::string>()(sip + "_" + curr_date);
	

	if (isExistOnHold(sip,std::to_string(hash))) 
	{
		return;
	}

	// нужна еще одна проверка т.к. коннект сбрасывается
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::addOperatorsOnHold");
		return;
	}

	// устанавливаем данные в operators_onhold
	std::string	query_insert = "insert into operators_ohhold (sip,hash,date_time_start,phone) values ('" + sip + "','"+ std::to_string(hash) + "','"+curr_date+ "','" + phone +"')";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::addOperatorsOnHold(const std::string &sip, const std::string &phone) -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::addOperatorsOnHold -> Data (addOperatorsOnHold) error -> query(" + query_insert + ")", &this->mysql);
		return;
	}	

	mysql_close(&this->mysql);
}


void SQL_REQUEST::SQL::disableOperatorsOnHold(const ACTIVE_SIP::OnHold &onhold)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::disableOperatorsOnHold");
		return;
	}

	std::string query = "update operators_ohhold set date_time_stop = '" + getCurrentDateTime() + "' where id = '" + std::to_string(onhold.id) 
																	  + "' and hash = '"+std::to_string(onhold.hash)+"'"
																	  + " and sip = '"+onhold.sip_number+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::disableOperatorsOnHold(const ACTIVE_SIP::OnHold &onhold) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::disableOperatorsOnHold -> Data (disableOperatorsOnHold) error -> query(" + query + ")", &this->mysql);
		return;
	};


	mysql_close(&this->mysql);
}

bool SQL_REQUEST::SQL::isExistOnHold(const std::string &sip, std::string hash)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::isExistOnHold");
		return true;
	}

	const std::string query = "select count(id) from operators_ohhold where sip = '" + sip + "' and hash = '" + hash+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add("SQL_REQUEST::SQL::isExistOnHold(const std::string &sip, std::string hash) -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistOnHold -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existOnHold;
	((std::stoi(row[0]) == 0) ? existOnHold = false : existOnHold = true);

	mysql_free_result(result);
	mysql_close(&this->mysql);

	return existOnHold;
}
