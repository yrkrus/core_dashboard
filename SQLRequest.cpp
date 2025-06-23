#include "SQLRequest.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "IVR.h"
#include "HouseKeeping.h"
#include <mysql/mysql.h>
#include <iterator>
#include <string>
#include <memory>
#include <list>


using namespace INTERNALFUNCTION;

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
		// ���� ���������� �� ������� � ������� ��������� �� ������
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
		// ���� ��� ����������� ���������� ���������� � �� ������� ��������� �� ������
		showErrorBD(METHOD_NAME + " -> Error: can't connect to database", &mysql);
		return;
	};

	mysql_set_character_set(&mysql, "utf8");
}

// ���� �� ������� � ��
bool SQL_REQUEST::SQL::isConnectedBD()
{
	// status = 0 ������ ������� ����
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


	// ���������
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


// ���������� ������ � ������� IVR
//void SQL_REQUEST::SQL::insertIVR(const char *phone, const char *time, std::string callerid)
//{	
//	if (!isConnectedBD()) {
//		showErrorBD(METHOD_NAME);
//		return;
//	}	
//	
//	// �������� ���� �� ����� ����� 	
//	if (isExistIVRPhone(phone))	{ // ����� ����������, ��������� ������
//		
//		std::string id = std::to_string(getLastIDphone(phone));
//		
//		updateIVR(id.c_str(), phone, time);
//		return;
//	}
//	else {		
//		std::string query = "insert into ivr (phone,waiting_time,trunk) values ('" + std::string(phone) + "','" + std::string(time) + "','" + callerid + "')";
//		
//		if (CONSTANTS::SAFE_LOG) {
//			if (CONSTANTS::LOG_MODE_DEBUG) 
//			{
//				LOG::LogToFile log(LOG::eLogType_DEBUG);
//				log.add(METHOD_NAME +" -> " + query);
//			}
//		}
//
//		if (mysql_query(&this->mysql, query.c_str()) != 0)
//		{
//			showErrorBD(METHOD_NAME+" -> Data (insertIVR) error -> query("+query+")", &this->mysql);
//		}	
//	}	
//
//	mysql_close(&this->mysql);
//}

// ���������� �� ����� ��� ����� � ������� IVR
//bool SQL_REQUEST::SQL::isExistIVRPhone(const char *phone)
//{
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return true;
//	}
//	
//	const std::string query = "select count(phone) from ivr where phone = '" 
//							  + std::string(phone) +"' and  date_time > '"
//							  + getCurrentDateTimeAfterMinutes(2)+"' and to_queue = '0' order by date_time desc";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str() ) != 0)	{
//		// ������ ������� ��� ���� ������		
//		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
//		return true;
//	}
//
//	// ���������
//	MYSQL_RES *result = mysql_store_result(&this->mysql);
//	MYSQL_ROW row = mysql_fetch_row(result);	
//	
//	bool existIvrPhone;
//	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;
//	
//	mysql_free_result(result);	
//
//	return existIvrPhone;	
//}

// ��������� ���������� ID �����������
//int SQL_REQUEST::SQL::getLastIDphone(const char *phone)
//{	
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return -1;
//	}
//
//	const std::string query = "select id from ivr where phone = "
//		+ std::string(phone) + " and date_time > '"
//		+ getCurrentStartDay() + "' order by date_time desc limit 1";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		// ������ ������� ��� ��� ������		
//		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
//		return -1;
//	}
//
//	MYSQL_RES *result = mysql_store_result(&this->mysql);
//	MYSQL_ROW row = mysql_fetch_row(result);
//	
//	int id = std::stoi(row[0]);
//
//	mysql_free_result(result);	
//
//	return id;
//}

// ���������� ������ � ������� IVR
//void SQL_REQUEST::SQL::updateIVR(const char *id,const char *phone, const char *time)
//{
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return;
//	}
//
//	std::string query = "update ivr set waiting_time = '" + std::string(time) + "' where phone = '" + std::string(phone) + "' and id ='"+std::string(id)+"'";
//	
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		showErrorBD(METHOD_NAME+" -> Data (updateIVR) error -> query(" + query + ")",&this->mysql);
//	};
//	
//
//	mysql_close(&this->mysql);
//}

// ���������� ������ � ������� QUEUE
void SQL_REQUEST::SQL::insertQUEUE(const char *queue, const char *phone, const char *time)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}	


	// �������� ���� �� ����� ����� 	
	if (isExistQUEUE(queue,phone))
	{ // ����� ����������, ��������� ������
	
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
				log.add(METHOD_NAME + " -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD(METHOD_NAME+" -> Data (insertQUEUE) error -> query(" + query + ")", &this->mysql);
		}		
	}

	mysql_close(&this->mysql);

}

// ���������� �� ����� ��� ����� � ������� QUEUE
bool SQL_REQUEST::SQL::isExistQUEUE(const char *queue, const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}	


	// ���������� ��������� ������� ��������	
	const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
		+ "' and phone = '" + std::string(phone) + "'"
		+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'"
		+ " and answered ='1' and fail='0' and sip<>'-1' and hash is NULL order by date_time desc limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	int countPhone = std::stoi(row[0]);
	mysql_free_result(result);

	if (countPhone >= 1)
	{
		return true;
	}
	else {
		// ��������� ����� � ������� ������ ��������� ������
		const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
			+ "' and phone = '" + std::string(phone) + "'"
			+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'" //��� ���� ��, �� ����� �� ������������� ��������� 15 ���
			//+ " and date_time > '" + getCurrentDateTime() + "'"
			+ " and answered ='0' and fail='0' and hash is NULL order by date_time desc limit 1";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add(METHOD_NAME + " -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			// ������ ������� ��� ���� ������		
			showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
			return true;
		}

		// ���������
		MYSQL_RES *result = mysql_store_result(&this->mysql);
		MYSQL_ROW row = mysql_fetch_row(result);
		int countPhone = std::stoi(row[0]);
		
		mysql_free_result(result);

		if (countPhone >= 1)
		{
			return true; 
		}
		else {
		
			// ��� ��������� ��������� ����������
			const std::string query = "select count(phone) from queue where number_queue = '" + std::string(queue)
				+ "' and phone = '" + std::string(phone) + "'"
				+ " and date_time > '" + getCurrentDateTimeAfterMinutes(60) + "'" //��� ���� ��, �� ����� �� ������������� ��������� 15 ���
				//+ " and date_time > '" + getCurrentDateTime() + "'"
				+ " and answered ='0' and fail='1' and hash is NULL order by date_time desc limit 1";

			if (CONSTANTS::SAFE_LOG)
			{
				if (CONSTANTS::LOG_MODE_DEBUG)
				{
					LOG::LogToFile log(LOG::eLogType_DEBUG);
					log.add(METHOD_NAME + " -> " + query);
				}
			}

			if (mysql_query(&this->mysql, query.c_str()) != 0)
			{
				// ������ ������� ��� ���� ������		
				showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
				return true;
			}

			// ���������
			MYSQL_RES *result = mysql_store_result(&this->mysql);
			MYSQL_ROW row = mysql_fetch_row(result);
			int countPhone = std::stoi(row[0]);
			
			mysql_free_result(result);

			if (countPhone >= 1)
			{
				return false; // ������� ��� ����� �����!!!
			}
			else 
			{			
			// �������� �� �����������, ����� ��� ��� ����������� ����� ���� ��� ���������� ���	
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
						log.add(METHOD_NAME + " -> " + query);
					}
				}

				if (mysql_query(&this->mysql, query.c_str()) != 0)
				{
					// ������ ������� ��� ���� ������		
					showErrorBD(METHOD_NAME +" -> query(" + query + ")", &this->mysql);
					return true;
				}
				// ���������
				MYSQL_RES *result = mysql_store_result(&this->mysql);
				MYSQL_ROW row = mysql_fetch_row(result);
				
				int countPhone = std::stoi(row[0]);
				mysql_free_result(result);

				if (countPhone >= 1)
				{
					return false;	// ���� ���� ������, ������ ��������� ������
				}
				
				return (countPhone == 0 ? false : true);
			}		
		}		
	}	
}

// ���������� ������ � ������� QUEUE
void SQL_REQUEST::SQL::updateQUEUE(const char *id, const char *phone, const char *time)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	std::string query = "update queue set waiting_time = '" + std::string(time) + "' where phone = '" + std::string(phone) + "' and id ='" + std::string(id) + "'";;

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (updateQUEUE) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// ��������� ���������� ID �����������
int SQL_REQUEST::SQL::getLastIDQUEUE(const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
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
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ��� ������		
		showErrorBD(METHOD_NAME+" -> query("+query+")",&this->mysql);
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	int count = std::stoi(row[0]);
	
	mysql_free_result(result);	

	return count;
}

// ���������� ������ ������� QUEUE � ��� � ��� ������ ������������� ��������
void SQL_REQUEST::SQL::updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// �������� ���� �� ����� ����� 	
	if (isExistQUEUE_SIP(phone))
	{ // ����� ����������, ��������� ������
		std::string id = std::to_string(getLastIDQUEUE(phone));

		std::string query = "update queue set sip = '" + std::string(sip) + "', talk_time = '"+ getTalkTime(talk_time) + "', answered ='1' where phone = '" + std::string(phone) + "' and id ='" + std::string(id) + "'";

		if (CONSTANTS::SAFE_LOG)
		{
			if (CONSTANTS::LOG_MODE_DEBUG)
			{
				LOG::LogToFile log(LOG::eLogType_DEBUG);
				log.add(METHOD_NAME + " -> " + query);
			}
		}

		if (mysql_query(&this->mysql, query.c_str()) != 0)
		{
			showErrorBD(METHOD_NAME+" -> Data (updateQUEUE_SIP) error -> query("+query+")", &this->mysql);
		};

		mysql_close(&this->mysql);		
	}	
}

// ���������� �� ����� ����� � ������� QUEUE ����� �������� sip ��������� ������� � �������� �����
bool SQL_REQUEST::SQL::isExistQUEUE_SIP(const char *phone)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
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
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existQueueSip;
	(std::stoi(row[0]) == 0 ? existQueueSip = false : existQueueSip = true);
	
	mysql_free_result(result);

	return existQueueSip;
	
}

//���������� ������ ����� ������ �� �������� ����� �������
//void SQL_REQUEST::SQL::updateQUEUE_fail(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list)
//{
//	
//	// ������ ������� ������ ������� �� �� ����� ������� ��� ����������
//	std::string list_phone;
//	
//	for (const auto &list : pacient_list) {
//		
//		if (list_phone.empty()) {
//			list_phone = "'"+list.phone+"'";
//		}
//		else {
//			list_phone = list_phone + ",'" + list.phone + "'";
//		}		
//	}	
//
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return;
//	}
//
//	// ��������� ������
//	std::string query = "update queue set fail = '1' where date_time > '" + getCurrentStartDay() + "' and answered = '0'"
//		+ " and sip = '-1' and phone not in ("+ list_phone +")";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		showErrorBD(METHOD_NAME+" -> Data (updateQUEUE_fail) error -> query(" + query + ")", &this->mysql);
//	};
//
//	mysql_close(&this->mysql);
//}

// ���������� ������ ����� ������ �� �������� ����� �������
void SQL_REQUEST::SQL::updateQUEUE_fail()
{
	// ��������� ������
	std::string query = "update queue set fail = '1' where date_time > '" + getCurrentDateTimeAfter20hours() + "' and answered = '0' and sip = '-1' ";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (updateQUEUE_fail) error -> query(" + query + ")", &this->mysql);
	};

	mysql_close(&this->mysql);
}

// ���������� ������ ����� � ��� ������ �� IVR ����� � �������
//void SQL_REQUEST::SQL::updateIVR_to_queue(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list)
//{
//	// ������ ������� ������ ������� ����� ������� ��� ����������
//	std::string list_phone;
//
//	for (const auto &list : pacient_list)
//	{
//		if (list_phone.empty())
//		{
//			list_phone = "'" + list.phone + "'";
//		}
//		else
//		{
//			list_phone = list_phone + ",'" + list.phone + "'";
//		}
//	}
//
//	// ��������� ������
//	std::string query = "update ivr set to_queue = '1' where date_time > '" + getCurrentDateTimeAfterMinutes(5)+"' and phone in(" + list_phone + ") and to_queue = '0'";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return;
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		showErrorBD(METHOD_NAME+" -> Data (updateIVR_to_queue) error -> query(" + query + ")", &this->mysql);
//	};
//
//	mysql_close(&this->mysql);
//}


// �������� ���� �� ������ ������� ��������� ����� 20:00
bool SQL_REQUEST::SQL::isExistQueueAfter20hours()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(phone) from queue where date_time > '"
		+ getCurrentDateTimeAfter20hours() + "' and sip = '-1' and answered = '0' and fail = '0' order by date_time desc ";
	
	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}
	
	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existQueueAfter20hours;
	(std::stoi(row[0]) == 0 ? existQueueAfter20hours = false : existQueueAfter20hours = true);

	mysql_free_result(result);	
	mysql_close(&this->mysql); 
			
	return existQueueAfter20hours;
}

// ���������� ���� hash ����� ������� ����������
//void SQL_REQUEST::SQL::updateQUEUE_hash(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list)
//{
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return;
//	}	
//
//	// ������ ������� ������ ������� ����� ������� ��� ����������
//	std::string list_phone;
//
//	for (const auto &list : pacient_list)
//	{
//		if (list_phone.empty())
//		{
//			list_phone = "'" + list.phone + "'";
//		}
//		else
//		{
//			list_phone = list_phone + ",'" + list.phone + "'";
//		}
//	} 
//
//	QUEUE_OLD::QueueBD_old queuebd;
//	
//	
//	const std::string query = "select id,phone,date_time from queue where date_time > '"
//		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL and phone not in("+ list_phone+")";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		// ������ ������� ��� ���� ������		
//		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
//		return;
//	}
//
//	// ���������
//	MYSQL_RES *result = mysql_store_result(&this->mysql);
//	MYSQL_ROW row; 
//	
//	while ((row = mysql_fetch_row(result)) != NULL)
//	{
//		QUEUE_OLD::BD bd;
//		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
//		{
//			
//			if (i == 0) {
//				bd.id = row[i];
//			}
//			else if (i == 1) {
//				bd.phone = row[i];
//			}
//			else if (i == 2) {
//				bd.date_time = row[i];
//			}			
//		}
//
//		bd.hash = std::hash<std::string>()(bd.phone+"_"+bd.date_time);
//		queuebd.list.push_back(bd);		
//	}	
//	
//	mysql_free_result(result);
//	
//	// ���������
//	for (const auto &list : queuebd.list)	{
//
//		std::string query = "update queue set hash = '" + std::to_string(list.hash)
//			+ "' where id ='" + list.id
//			+ "' and phone ='" + list.phone
//			+ "' and date_time = '" + list.date_time + "'";
//
//		if (CONSTANTS::SAFE_LOG)
//		{
//			if (CONSTANTS::LOG_MODE_DEBUG)
//			{
//				LOG::LogToFile log(LOG::eLogType_DEBUG);
//				log.add(METHOD_NAME + " -> " + query);
//			}
//		}
//
//		if (!isConnectedBD())
//		{
//			showErrorBD(METHOD_NAME);
//			return;
//		}
//
//		if (mysql_query(&this->mysql, query.c_str()) != 0)
//		{
//			showErrorBD(METHOD_NAME+" -> Data (updateQUEUE_hash) error -> query(" + query + ")", &this->mysql);
//		};
//
//	};
//	mysql_close(&this->mysql); 
//}


// �������� ���� �� ������ ������� ����� ���������� ������ �������� ����� ���� ��� �������� ���� �� �����
bool SQL_REQUEST::SQL::isExistAnsweredAfter20hours()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(id) from queue where date_time > '"
		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";


	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existAnsweredAfter20;
	(std::stoi(row[0]) == 0 ? existAnsweredAfter20 = false : existAnsweredAfter20 = true);

	mysql_free_result(result);
	mysql_close(&this->mysql); 

	return existAnsweredAfter20;
}

// ���������� ������ ����� �������� ��������� � ���� �� �����, � ������ ��� ��� ��������� �� ������������
//void SQL_REQUEST::SQL::updateAnswered_fail()
//{
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return;
//	}	
//
//	QUEUE_OLD::QueueBD_old queuebd;
//
//	const std::string query = "select id,phone,date_time from queue where date_time > '"
//		+ getCurrentStartDay() + "' and answered = '1' and fail = '0' and hash is NULL";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		// ������ ������� ��� ���� ������		
//		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
//		return;
//	}
//
//	// ���������
//	MYSQL_RES *result = mysql_store_result(&this->mysql);
//	MYSQL_ROW row;
//
//	while ((row = mysql_fetch_row(result)) != NULL)
//	{
//		QUEUE_OLD::BD bd;
//		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
//		{
//
//			if (i == 0)
//			{
//				bd.id = row[i];
//			}
//			else if (i == 1)
//			{
//				bd.phone = row[i];
//			}
//			else if (i == 2)
//			{
//				bd.date_time = row[i];
//			}
//		}
//
//		bd.hash = std::hash<std::string>()(bd.phone + "_" + bd.date_time);
//		queuebd.list.push_back(bd);
//	}
//
//	mysql_free_result(result);
//
//	// ���������
//	for (const auto &list : queuebd.list)
//	{
//
//		std::string query = "update queue set hash = '" + std::to_string(list.hash)
//			+ "' where id ='" + list.id
//			+ "' and phone ='" + list.phone
//			+ "' and date_time = '" + list.date_time + "'";
//
//		if (CONSTANTS::SAFE_LOG)
//		{
//			if (CONSTANTS::LOG_MODE_DEBUG)
//			{
//				LOG::LogToFile log(LOG::eLogType_DEBUG);
//				log.add(METHOD_NAME + " -> " + query);
//			}
//		}
//
//		if (!isConnectedBD())
//		{
//			showErrorBD(METHOD_NAME);
//			return;
//		}
//
//		if (mysql_query(&this->mysql, query.c_str()) != 0)
//		{
//			showErrorBD(METHOD_NAME+" -> Data(updateAnswered_fail) error -> query(" + query + ")", &this->mysql);
//		};
//
//	};
//	mysql_close(&this->mysql);
//}


// ���������� sip ������ ��������� + ��� �������
void SQL_REQUEST::SQL::insertOperatorsQueue(const char *sip, const char *queue)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
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
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}
	

	mysql_close(&this->mysql);
}

// ������� ������� operators_queue
void SQL_REQUEST::SQL::clearOperatorsQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}
	
	std::string query = "delete from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}	

	mysql_close(&this->mysql);
}

// �������� ���������� �� ����� sip+������� � ��
bool SQL_REQUEST::SQL::isExistOperatorsQueue(const char *sip, const char *queue)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(id) from operators_queue where sip = '"+ std::string(sip) + "' and queue = '"+std::string(queue)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);	

	bool existOperatos;
	std::stoi(row[0]) == 0 ? existOperatos = false : existOperatos = true;
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return existOperatos;

}

// �������� ���������� �� ���� 1 ������ � �� sip+�������
bool SQL_REQUEST::SQL::isExistOperatorsQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(id) from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+"->query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	
	bool existOperatorsQueue;
	(std::stoi(row[0]) == 0 ? existOperatorsQueue = false : existOperatorsQueue = true);

	mysql_free_result(result);
	mysql_close(&this->mysql);

	return existOperatorsQueue;		
}

// �������� sip ������ ��������� �� ���� ��������
void SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}
	

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (deleteOperatorsQueue) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);
}

// �������� sip ������ ��������� � ���������� �������
void SQL_REQUEST::SQL::deleteOperatorsQueue(const std::string &sip, const std::string &queue)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	std::string query = "delete from operators_queue where sip = '" + sip + "' and queue = '" + queue + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (deleteOperatorsQueue(2 param)) error -> query(" + query + ")", &this->mysql);
	}

	mysql_close(&this->mysql);

}


// �������� ������� sip + �������
void SQL_REQUEST::SQL::checkOperatorsQueue(const std::vector<ACTIVE_SIP_old::Operators> &list_operators)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ������ �� ��
	ACTIVE_SIP_old::Operators curr_list_operator;
	std::vector<ACTIVE_SIP_old::Operators> list_operators_bd;

	const std::string query = "select sip,queue from operators_queue";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
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

	// �������� ��������� �� ������ � ������� �� ��
	for (const auto &curr_list : list_operators_bd)	{
		bool isExistSip{ true };	// ������� ��� sip �� ��������� ����������
		bool isExistQueue{ true };	// ������� ��� queue �� ��������� ����������

		for (const auto &memory_list : list_operators) {
			if (curr_list.sip_number == memory_list.sip_number) {
				isExistSip = true;
				
				// �������� ���� �� ����� �������
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
				// ��� sip, ���� ������� �� ��
				isExistSip = false;
			}
		}
		
		// ��� ������ ����� ������� �� ��
		if (isExistSip) {
			if (!isExistQueue) {
				// ������� sip + ������� ����������
				SQL_REQUEST::SQL base;
				
				if (base.isConnectedBD())
				{
					base.deleteOperatorsQueue(curr_list.sip_number, curr_list.queue[0]);
				}
			
			}
		}
		else {
			// ������� ���� sip
			SQL_REQUEST::SQL base;

			if (base.isConnectedBD())
			{
				base.deleteOperatorsQueue(curr_list.sip_number);
			}
		}
	}		
}




// ������� ����� ��������� �� ����� IVR
int SQL_REQUEST::SQL::getIVR_totalCalls()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return 0;
	}

	const std::string query = "select count(phone) from ivr where date_time > '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return 0;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); 

	return std::stoi(row[0]);
}

// ������� ����� ��������� �� ����� IVR (����� �� trunk)
//int SQL_REQUEST::SQL::getIVR_totalCalls(const IVR_OLD::CallerID &trunk)
//{
//	if (!isConnectedBD())
//	{
//		showErrorBD(METHOD_NAME);
//		return 0;
//	}
//
//	const std::string query = "select count(phone) from ivr where trunk ='" + IVR_OLD::getCallerID(trunk) + "'  and date_time > '" + getCurrentStartDay() + "'";
//
//	if (CONSTANTS::SAFE_LOG)
//	{
//		if (CONSTANTS::LOG_MODE_DEBUG)
//		{
//			LOG::LogToFile log(LOG::eLogType_DEBUG);
//			log.add(METHOD_NAME + " -> " + query);
//		}
//	}
//
//	if (mysql_query(&this->mysql, query.c_str()) != 0)
//	{
//		// ������ ������� ��� ���� ������		
//		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
//		return 0;
//	}
//
//	// ���������
//	MYSQL_RES *result = mysql_store_result(&this->mysql);
//	MYSQL_ROW row = mysql_fetch_row(result);
//	mysql_free_result(result);
//
//	mysql_close(&this->mysql); // ��� ��������?
//
//	return std::stoi(row[0]);;
//}

// ������� ����� �������� � ������� �����������
int SQL_REQUEST::SQL::getQUEUE_Calls(bool answered)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
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
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return 0;
	}
}
	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); // ��� ��������?

	return std::stoi(row[0]);
}

// ������� ����� ���� � �������
int SQL_REQUEST::SQL::getQUEUE_Calls()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return 0;
	}		
	 const std::string query = "select count(phone) from queue where date_time > '" + getCurrentStartDay()+"'";

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return 0;
	}
	
	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);
	mysql_free_result(result);

	mysql_close(&this->mysql); // ��� ��������?

	return std::stoi(row[0]);
}

// �������� ���� ����� ������� ���  �����\������ �� �������
bool SQL_REQUEST::SQL::remoteCheckNewCommads()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(id) from remote_commands";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);			

	bool existNewCommand;
	((std::stoi(row[0]) == 0) ? existNewCommand = false : existNewCommand = true);
	
	mysql_free_result(result);
	mysql_close(&this->mysql);
	
	return existNewCommand;
}


// ��������� ������� ��������� ������
void SQL_REQUEST::SQL::createListRemoteCommands(std::vector<REMOTE_COMMANDS::R_Commands> &list)
{	
	
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}	
	// ��������� ���� 
	const std::string query = "select id,sip,command,ip,user_id,user_login_pc,pc from remote_commands";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)	{
		
		
		int id;						// id ������� (��� �������� ������ � �������)
		std::string sip;			// sip ������������������ �������
		LOG::Log command;			// ���� ������� (int)
		std::string ip;				// ip � �������� ������ �������
		int user_id;				// id ������������ �� ��
		std::string user_login_pc;	// ����� ������� �� �� � �������� ������ �������
		std::string pc;				// ��� �� � �������� ��������� ��������


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

		// ������� ������ ������� � ������ ���� ���������� ������
		list.push_back({id,sip,command,ip,user_id,user_login_pc,pc});
	}


	mysql_free_result(result);	
	mysql_close(&this->mysql);
	
}

// ������ ��������� �������
void SQL_REQUEST::SQL::startRemoteCommand(int id, std::string sip, LOG::Log command, int user_id)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ��������� �������
	std::string responce_now;	
	
	// � ����� ������� ���������
	CONSTANTS::AsteriskQueue curr_queue;


	switch (command)
	{
		// ����������� � ������� 5000 ��� 5050	
		case LOG::Log::Log_add_queue_5000 ... LOG::Log::Log_add_queue_5050 :  
		{			
			std::string responce = CONSTANTS::cRemoteCommandResponseAdd;
			
			// �������� % �� ������ ���
			std::string repl_sip = "%sip";			
			 
			size_t position = responce.find(repl_sip);

			while (position != std::string::npos) 
			{
				responce.replace(position, repl_sip.length(), sip);
				position = responce.find(repl_sip);
			}
			
			std::string repl_queue = "%queue";
			position = responce.find(repl_queue);
			
			//� ����� ������� ��������� ���������
			if (command == LOG::Log::Log_add_queue_5000) 
			{
				curr_queue = CONSTANTS::AsteriskQueue::main;
			}
			else if (command == LOG::Log::Log_add_queue_5050) 
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
		// ���������� � ������� 5000+5050
		case LOG::Log::Log_add_queue_5000_5050: {		 
			
			LOG::Log command = LOG::Log::Log_add_queue_5000;			
			// ������ 5000
			startRemoteCommand(id,sip, command, user_id);


			command = LOG::Log::Log_add_queue_5050;
			// ������ 5050
			startRemoteCommand(id, sip, command, user_id);
			
			break;
		}	
		case LOG::Log::Log_del_queue_5000 ... LOG::Log::Log_del_queue_5050:
			{

			std::string responce = CONSTANTS::cRemoteCommandResponseDel;

			// �������� % �� ������ ���
			std::string repl_sip = "%sip";

			size_t position = responce.find(repl_sip);
			
			responce.replace(position, repl_sip.length(), sip);			

			std::string repl_queue = "%queue";
			position = responce.find(repl_queue);

			//� ����� ������� ��������� ���������
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
			// �������� �� ������� 5000+5050
		case LOG::Log::Log_del_queue_5000_5050:
		{

			LOG::Log command = LOG::Log::Log_del_queue_5000;
			// ������ 5000
			startRemoteCommand(id, sip, command, user_id);


			command = LOG::Log::Log_del_queue_5050;
			// ������ 5050
			startRemoteCommand(id, sip, command, user_id);

			break;
		}
			// �����
		case LOG::Log::Log_home ... LOG::Log::Log_callback : {
			// �� ��� ��� ������ �� �������� �� ���� � ����� ������� ��������� ������� �� ���� ��������
			LOG::Log command = LOG::Log::Log_del_queue_5000_5050;			
			startRemoteCommand(id, sip, command, user_id);
		}
	}
	
}

// �������� ������� ����������� �������
void SQL_REQUEST::SQL::deleteRemoteCommand(int id)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	std::string query = "delete from remote_commands where id = '"+std::to_string(id)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> " + query, &this->mysql);
	}

	mysql_close(&this->mysql);
}

// ���������� �������� ������� ���������
void SQL_REQUEST::SQL::updateStatusOperators(int user_id, REMOTE_COMMANDS::ecStatusOperators status)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}
	
	std::string query = "update operators set status = '" + std::to_string(getStatusOperators(status)) + "' where user_id = '" + std::to_string(user_id) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> " + query, &this->mysql);
	};

	
	mysql_close(&this->mysql);
	
}

// �������� ���� � ��
void SQL_REQUEST::SQL::addLog(LOG::Log command, int base_id)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}


	// ������ ��� ������ �� ������������ ��� �����������
	const std::string query = "select sip,ip,user_id,user_login_pc,pc from remote_commands where id = '"+ std::to_string(base_id)+"' limit 1";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> " + query, &this->mysql);
		return;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;
	
	std::string sip;			// sip ������������������ �������	
	std::string ip;				// ip � �������� ������ �������
	int user_id;				// id ������������ �� ��
	std::string user_login_pc;	// ����� ������� �� �� � �������� ������ �������
	std::string pc;				// ��� �� � �������� ��������� ��������

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
	
	// ������������� ������ � ���
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
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertIVR) error -> query(" + query_insert + ")", &this->mysql);
	}
	
	mysql_close(&this->mysql);
}


void SQL_REQUEST::SQL::execTaskQueue()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� ������ 
	const std::string query = "select * from queue where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}	

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::Queue_old> listQueue;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::Queue_old queue;
		
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
			else if (i == 6) { // talk_time (����� ���� NULL!)
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
			else if (i == 9) { // hash (����� ���� NULL!)
				if (row[i]) {
					queue.hash = string_to_size_t(row[i]);
				}			
			}
		}

		listQueue.emplace_back(queue);
	}

	if (!listQueue.empty()) { 
		std::cout << "HouseKeeping.Queue work...\n";

		// ������������� 1 ���������� ��� ��� ? \ ��� �� 100 ���� ��������..
		SQL_REQUEST::SQL base;

		for (auto &list : listQueue)
		{		
			if (base.insertDataTaskQueue(list)) {
				// ������� ������� �����������
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
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� ������ 
	const std::string query = "select * from logging where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
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
		// ������������� 1 ���������� ��� ��� ? \ ��� �� 100 ���� ��������..
		SQL_REQUEST::SQL base;

		for (auto &list : listLogging)
		{
			if (base.insertDataTaskLogging(list))
			{
				// ������� ������� �����������
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
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� ������ 
	const std::string query = "select * from ivr where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::IVR_> listIvr;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::IVR_ ivr;

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

		// ������������� 1 ���������� ��� ��� ? \ ��� �� 100 ���� ��������..
		SQL_REQUEST::SQL base;

		for (auto &list : listIvr)
		{
			if (base.insertDataTaskIvr(list))
			{
				// ������� ������� �����������
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
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� ������ 
	const std::string query = "select * from operators_ohhold where date_time_start < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
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
		// ������������� 1 ���������� ��� ��� ? \ ��� �� 100 ���� ��������..
		SQL_REQUEST::SQL base;

		for (auto &list : listOnHold)
		{
			if (base.insertDataTaskOnHold(list))
			{
				// ������� ������� �����������
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


void SQL_REQUEST::SQL::execTaskSmsSending()
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� ������ 
	const std::string query = "select * from sms_sending where date_time < '" + getCurrentStartDay() + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME + " -> query(" + query + ")", &this->mysql);
		return;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;

	std::vector<HOUSEKEEPING::SmsSending> listSmsSending;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		HOUSEKEEPING::SmsSending smsSending;

		for (unsigned int i = 0; i < mysql_num_fields(result); ++i)
		{
			if (i == 0)			// id
			{
				smsSending.id = std::atoi(row[i]);
			}
			else if (i == 1)	// user_id
			{
				smsSending.user_id = std::atoi(row[i]);
			}
			else if (i == 2)   // date_time
			{
				smsSending.date_time = row[i];
			}
			else if (i == 3)   // phone
			{
				smsSending.phone = row[i];
			}			
			else if (i == 4) // message
			{
				smsSending.message	= row[i];
			}
			else if (i == 5) // sms_id
			{
				smsSending.sms_id = string_to_size_t(row[i]);
			} 
			else if (i == 6) // status
			{
				if (row[i]) 
				{
					smsSending.status = row[i];
				}				
			}
			else if (i == 7) // user_login_pc
			{
				smsSending.user_login_pc = row[i];
			}
			else if (i == 8) // count_real_sms
			{
				smsSending.count_real_sms = std::atoi(row[i]);
			}
			else if (i == 9) // sms_type
			{
				smsSending.sms_type = std::atoi(row[i]);
			}
		}

		listSmsSending.emplace_back(smsSending);
	}

	if (!listSmsSending.empty())
	{
		std::cout << "HouseKeeping.SmsSending work...\n";
		// ������������� 1 ���������� ��� ��� ? \ ��� �� 100 ���� ��������..
		SQL_REQUEST::SQL base;

		for (const auto &list : listSmsSending)
		{
			if (base.insertDataTaskSmsSending(list))
			{
				// ������� ������� �����������
				base.deleteDataTaskSmsSending(list.id);
			}
		}
		std::cout << "HouseKeeping.SmsSending work DONE!\n";

		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
	else
	{
		mysql_free_result(result);
		mysql_close(&this->mysql);
	}
}

bool SQL_REQUEST::SQL::insertDataTaskQueue(HOUSEKEEPING::Queue_old &queue)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query_insert;
	
	if (queue.sip != -1) {	 // �� ������ ���� ��� NULL �����

		// ������������� ������ � history_queue
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
	else {					 // �� ������ ���� ���� NULL ����

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
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertDataTaskQueue) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);
	
	return true;
}


bool SQL_REQUEST::SQL::deleteDataTaskQueue(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query = "delete from queue where id = '"+std::to_string(ID)+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (deleteDataTaskQueue) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskLogging(HOUSEKEEPING::Logging &logging)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}	

   // ������������� ������ � history_logging
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
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertDataTaskLogging) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::deleteDataTaskLogging(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query = "delete from logging where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (deleteDataTaskLogging) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskIvr(HOUSEKEEPING::IVR_ &ivr)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	// ������������� ������ � history_ivr
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
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertDataTaskIvr) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::deleteDataTaskIvr(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query = "delete from ivr where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (deleteDataTaskIvr) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskOnHold(HOUSEKEEPING::OnHold &onHold)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	// ������������� ������ � history_ivr
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
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (insertDataTaskOnHold) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}


bool SQL_REQUEST::SQL::deleteDataTaskOnHold(int ID)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query = "delete from operators_ohhold where id = '" + std::to_string(ID) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME + " -> Data (deleteDataTaskOnHold) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}

bool SQL_REQUEST::SQL::insertDataTaskSmsSending(const HOUSEKEEPING::SmsSending &_smsSending)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}
	std::string	query_insert;

	// ������������� ������ � history_sms_sending
	if (_smsSending.status != "") 
	{
		query_insert = "insert into history_sms_sending (id,user_id,date_time,phone,message,sms_id,status,user_login_pc,count_real_sms,sms_type) values ('" +
			std::to_string(_smsSending.id) +
			"','" + std::to_string(_smsSending.user_id) +
			"','" + _smsSending.date_time +
			"','" + _smsSending.phone +
			"','" + _smsSending.message +
			"','" + std::to_string(_smsSending.sms_id) +
			"','" + _smsSending.status +
			"','" + _smsSending.user_login_pc +
			"','" + std::to_string(_smsSending.count_real_sms) +
			"','" + std::to_string(_smsSending.sms_type) + "')";
	} 
	else  // ���� null ���� �� status
	{
		query_insert = "insert into history_sms_sending (id,user_id,date_time,phone,message,sms_id,user_login_pc,count_real_sms,sms_type) values ('" +
			std::to_string(_smsSending.id) +
			"','" + std::to_string(_smsSending.user_id) +
			"','" + _smsSending.date_time +
			"','" + _smsSending.phone +
			"','" + _smsSending.message +
			"','" + std::to_string(_smsSending.sms_id) +			
			"','" + _smsSending.user_login_pc +
			"','" + std::to_string(_smsSending.count_real_sms) +
			"','" + std::to_string(_smsSending.sms_type) + "')";
	}	

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME + " -> Data (insertDataTaskSmsSending) error -> query(" + query_insert + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}


bool SQL_REQUEST::SQL::deleteDataTaskSmsSending(int _id)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return false;
	}

	std::string query = "delete from sms_sending where id = '" + std::to_string(_id) + "'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME + " -> Data (deleteDataTaskSmsSending) error -> query(" + query + ")", &this->mysql);
		return false;
	}

	mysql_close(&this->mysql);

	return true;
}



void SQL_REQUEST::SQL::updateOperatorsOnHold(ACTIVE_SIP_old::Parsing_old *list)
{
	typedef std::vector<ACTIVE_SIP_old::Operators>	operators;
	typedef std::vector<ACTIVE_SIP_old::OnHold>		operators_onhold;
	
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������ ��� sip ��������� ������� ��������� �� �� � ������� onHold	
	auto onHold = createOnHoldSip();
	//auto onHold = createOnHoldSip();	

	if (onHold->size() >= 4) {
		std::cout << "test";
		/*
		
		id |sip|date_time_start    |date_time_stop     |hash                |phone       |
		---+---+-------------------+-------------------+--------------------+------------+
		416|508|2024-11-22 14:55:35|                   |11337464765379681510|+79093910402|
		417|514|2024-11-22 14:57:33|2024-11-22 14:58:14|13031441188243052791|+79064103692|
		418|507|2024-11-22 14:58:53|                   |7060977062918673759 |+79608957997|
		419|508|2024-11-22 14:58:58|                   |12630799220711158958|+79093910402|
		420|507|2024-11-22 14:58:58|                   |12077553184312573086|+79608957997|

			Line QUEUE is (9)
			queue           phone            wait time
			5000     >>     +79610731199     (00:01:05)
			5000     >>     +79618438788     (00:04:08)
			5000     >>     +79093910402     (00:05:01)
			5000     >>     +79064103692     (00:02:42)
			5000     >>     +79889863771     (00:04:00)
			5000     >>     +79377052662     (00:00:49)
			5000     >>     +79610758571     (00:03:39)
			5000     >>     +79608957997     (00:03:26)
			5000     >>     +78443420118     (00:01:50)
			Line IVR is (5)
			trunk           phone            wait time
			COMAGIC  >>     +79093941546     (00:00:33)
			COMAGIC  >>     +79696522569     (00:00:45)
			STS      >>     +78443385708     (00:00:05)
			220220   >>     +79275018420     (00:00:04)
			220220   >>     +79951343146     (00:00:33)
			Line Active SIP is (9)
			sip             phone            talk time
			507 (OnHold)    +79608957997     (00:02:38)
			502      >>     +79889863771     (00:03:12)
			526      >>     +79377052662     (00:00:01)
			514      >>     +79064103692     (00:01:55)
			506      >>     +79610758571     (00:02:51)
			512 (OnHold)    +79618438788     (00:03:20)
			519      >>     +78443420118     (00:01:03)
			544      >>     +79610731199     (00:00:17)
			508 (OnHold)    +79093910402     (00:04:07)

		*/

	}

	// ���������
	if (!onHold->empty()) {
		// ��������� ���� �� ������ ��������� � onHold
		operators curr_list_operators = list->getListOperators();
		
		
		// TODO ����� ��� ���� ����� ������ ����� �� ����� ���������� ������ ���� ���������
		if (CONSTANTS::LOG_MODE_INFO)
		{
			LOG::LogToFile log(LOG::eLogType_INFO);
			log.add(onHold, &curr_list_operators);
		}


		// ���������� �� ������ ����� ���� ������ �� onHold ��������, �.�. ��������� onHold, � � ������ �� ��� �������
		bool needCheckOnHold{ false };

		// ��������� ������� ������� ������� ��� ���� � onHold ���������		
		if (!curr_list_operators.empty())
		{				
			bool isExistOnHold{ true };

			// �������� �������� ��� ������������� � onHold ��� ��� ���
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
						
						// �������� �������� ���� � onHold ������ ����� ��� �� �������� ���� ������
						if (!isExistOnHold)
						{
							SQL base;
							base.disableOperatorsOnHold(*operators_hold);
							
							// ������� � ���� ��� ����� ������ �� onHold
							needCheckOnHold = true;							
						} 				
					}					
				}				
			}			


			// ����� ����� onHold ���������, ��������� � ��, �� ������� �������� delOnHold
			if (needCheckOnHold) 
			{
				auto onHoldNeedCheck = createOnHoldSip();

				if (!isExistNewOnHoldOperators(onHoldNeedCheck, curr_list_operators))
				{					
					auto new_list = createNewOnHoldOperators(onHoldNeedCheck, curr_list_operators);

					for (auto iter = new_list->begin(); iter != new_list->end(); ++iter)
					{
						SQL base;
						base.addOperatorsOnHold(iter->first, iter->second);
					}

					//delete new_list;
				}

				//delete onHoldNeedCheck;
			}
			else {
				if (!isExistNewOnHoldOperators(onHold, curr_list_operators))
				{				
					auto new_list = createNewOnHoldOperators(onHold, curr_list_operators);

					for (auto iter = new_list->begin(); iter != new_list->end(); ++iter)
					{
						SQL base;
						base.addOperatorsOnHold(iter->first,iter->second);
					}

					//delete new_list;
				}
			}
		}
		else
		{ // ������� ������ �.�. ���������� ��� ��������
			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
			{
				SQL base;
				base.disableOperatorsOnHold(*operators_hold);
			}
		}				
		
	}
	else {	// ������ ��������� ��� � ��� ������ �� ������ ������ ���� � ��������� � ��
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

	// delete onHold;
}

std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold>> SQL_REQUEST::SQL::createOnHoldSip()
{
	auto listHold = std::make_shared<std::vector<ACTIVE_SIP_old::OnHold>>();

	
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return nullptr;
	}

	// ������ ��� ������ 
	const std::string query = "select operators_ohhold.id, operators_ohhold.sip, operators_ohhold.date_time_start, operators_ohhold.date_time_stop, operators_ohhold.hash, queue.phone from queue inner join operators_ohhold on queue.sip = operators_ohhold.sip where queue.hash is NULL and operators_ohhold.date_time_stop is NULL";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������		
		showErrorBD(METHOD_NAME+" -> query(" + query + ")", &this->mysql);
		return nullptr;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row;	

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		ACTIVE_SIP_old::OnHold hold;

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
		showErrorBD(METHOD_NAME);
		return;
	}
	
	std::string curr_date = getCurrentDateTime();
	size_t hash = std::hash<std::string>()(sip + "_" + curr_date);
	

	if (isExistOnHold(sip,std::to_string(hash))) 
	{
		return;
	}

	// ����� ��� ���� �������� �.�. ������� ������������
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return;
	}

	// ������������� ������ � operators_onhold
	std::string	query_insert = "insert into operators_ohhold (sip,hash,date_time_start,phone) values ('" + sip + "','"+ std::to_string(hash) + "','"+curr_date+ "','" + phone +"')";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query_insert);
		}
	}

	if (mysql_query(&this->mysql, query_insert.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (addOperatorsOnHold) error -> query(" + query_insert + ")", &this->mysql);
		return;
	}	

	mysql_close(&this->mysql);
}


void SQL_REQUEST::SQL::disableOperatorsOnHold(const ACTIVE_SIP_old::OnHold &onhold)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
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
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		showErrorBD(METHOD_NAME+" -> Data (disableOperatorsOnHold) error -> query(" + query + ")", &this->mysql);
		return;
	};


	mysql_close(&this->mysql);
}

bool SQL_REQUEST::SQL::isExistOnHold(const std::string &sip, std::string hash)
{
	if (!isConnectedBD())
	{
		showErrorBD(METHOD_NAME);
		return true;
	}

	const std::string query = "select count(id) from operators_ohhold where sip = '" + sip + "' and hash = '" + hash+"'";

	if (CONSTANTS::SAFE_LOG)
	{
		if (CONSTANTS::LOG_MODE_DEBUG)
		{
			LOG::LogToFile log(LOG::eLogType_DEBUG);
			log.add(METHOD_NAME + " -> " + query);
		}
	}

	if (mysql_query(&this->mysql, query.c_str()) != 0)
	{
		// ������ ������� ��� ���� ������		
		showErrorBD(METHOD_NAME + " -> query(" + query + ")", &this->mysql);
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(&this->mysql);
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existOnHold;
	((std::stoi(row[0]) == 0) ? existOnHold = false : existOnHold = true);

	mysql_free_result(result);
	mysql_close(&this->mysql);

	return existOnHold;
}
