#include "SQLRequest.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "IVR.h"
#include "HouseKeeping.h"
#include <mysql/mysql.h>
#include <iterator>
#include <string>
#include <memory>



SQL_REQUEST::SQL::SQL()
{	
	createMySQLConnect(this->mysql);
}

void SQL_REQUEST::SQL::createMySQLConnect(MYSQL &mysql)
{

	const char *host = CONSTANTS::cHOST.c_str();
	const char *login = CONSTANTS::cLOGIN.c_str();
	const char *pwd = CONSTANTS::cPASSWORD.c_str();
	const char *bd = CONSTANTS::cBD.c_str();

	if (mysql_init(&mysql) == nullptr)
	{
		// Если дескриптор не получен — выводим сообщение об ошибке
		std::cerr << "Error: can't create MySQL-descriptor\n";
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

	if (mysql_query(&this->mysql, query.c_str() ) != 0)	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistIVRPhone -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);	
	mysql_free_result(result);	


	return ( std::stoi(row[0]) == 0 ? false : true);	
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

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что нет записи		
		showErrorBD("SQL_REQUEST::SQL::getLastIDphone -> query(" + query + ")", &this->mysql);
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);	

	return std::stoi(row[0]);
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

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	if (std::stoi(row[0]) >= 1)
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


		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			// ошибка считаем что есть запись		
			showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
			return true;
		}

		// результат
		MYSQL_RES *result = mysql_store_result(&this->mysql);
		MYSQL_ROW row = mysql_fetch_row(result);
		mysql_free_result(result);

		if (std::stoi(row[0]) >= 1)
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


			if (mysql_query(&this->mysql, query.c_str()) != 0)
			{
				// ошибка считаем что есть запись		
				showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
				return true;
			}

			// результат
			MYSQL_RES *result = mysql_store_result(&this->mysql);
			MYSQL_ROW row = mysql_fetch_row(result);
			mysql_free_result(result);

			if (std::stoi(row[0]) >= 1)
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

				if (mysql_query(&this->mysql, query.c_str()) != 0)
				{
					// ошибка считаем что есть запись		
					showErrorBD("SQL_REQUEST::SQL::isExistQUEUE -> query(" + query + ")", &this->mysql);
					return true;
				}
				// результат
				MYSQL_RES *result = mysql_store_result(&this->mysql);
				MYSQL_ROW row = mysql_fetch_row(result);
				mysql_free_result(result);

				if (std::stoi(row[0]) >= 1)
				{
					return false;	// если есть запись, значит повторный звонок
				}
				
				return (std::stoi(row[0]) == 0 ? false : true);			
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

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что нет записи		
		showErrorBD("SQL_REQUEST::SQL::getLastIDQUEUE -> query("+query+")",&this->mysql);
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);	

	return std::stoi(row[0]);
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

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQUEUE_SIP -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	return (std::stoi(row[0]) == 0 ? false : true);
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
	
	
	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistQueueAfter20hours -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);	

	mysql_close(&this->mysql); // добавил! 01.06.2024

	return (std::stoi(row[0]) == 0 ? false : true);

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


	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistAnsweredAfter20hours -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return (std::stoi(row[0]) == 0 ? false : true);


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


	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return (std::stoi(row[0]) == 0 ? false : true);

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


	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::isExistOperatorsQueue -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql);

	return (std::stoi(row[0]) == 0 ? false : true);
}

// удаление sip номера оператора из всех очередей
void SQL_REQUEST::SQL::deleteOperatorsQueue(std::string sip)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue()");
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "'";

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue -> Data (deleteOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// удаление sip номера оператора и конкретной очереди
void SQL_REQUEST::SQL::deleteOperatorsQueue(std::string sip, std::string queue)
{
	if (!isConnectedBD())
	{
		showErrorBD("SQL_REQUEST::SQL::deleteOperatorsQueue(2 param)");
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "' and queue = '" + queue + "'";

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


	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ошибка считаем что есть запись		
		showErrorBD("SQL_REQUEST::SQL::remoteCheckNewCommads -> query(" + query + ")", &this->mysql);
		return true;
	}

	// результат
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql);

	return (std::stoi(row[0]) == 0 ? false : true);
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
			//position = responce.find(repl_sip);
			
			//while (position != std::string::npos)
			//{}

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
					queue.hash = std::atoi(row[i]);
				}			
			}
		}

		listQueue.emplace_back(queue);
	}

	if (!listQueue.empty()) { 
		
		// перекидывание 1 транзакции хза раз ? \ или по 100 надо подумать..
		for (auto &list : listQueue)
		{
			SQL_REQUEST::SQL base;
			base.insertDataTaskQueue(list);



		}	
	}
}

void SQL_REQUEST::SQL::execTaskLogging()
{
}

void SQL_REQUEST::SQL::execTaskIvr()
{
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

	

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD("SQL_REQUEST::SQL::insertDataTaskQueue -> Data (insertDataTaskQueue) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);
	
	return true;
}
