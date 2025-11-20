/**
 * @file 	ISQLConnect.h
 * @brief  	класс для SQL запросов
 * @
*/

#ifndef ISQLCONNECT_H
#define ISQLCONNECT_H

#include <mysql/mysql.h>
#include <string>
#include <memory>
#include "../system/AUTH.h"

#define HOST AUTH::MYSQL::HOST.c_str();
#define BASE AUTH::MYSQL::BD.c_str();
#define LOGIN AUTH::MYSQL::LOGIN.c_str();
#define PWD AUTH::MYSQL::PASSWORD.c_str();


class ISQLConnect 
{
private:
	MYSQL	m_mysql;
	bool	m_connected;
	bool	m_initialized;  // был ли вызван mysql_init

	// подключаемся к БД MySQL
	bool ConnectInternal(std::string &_errorDescription);

public:
	ISQLConnect(bool _autoConnect = false);

	virtual ~ISQLConnect();	
	bool IsConnected() const;

	bool Connect(std::string &_errorDescription);
	void Disconnect();

	bool Request(const std::string &_request, std::string &_errorDescription);
	bool Request(const std::string &_request);

	// доступ к сырым данным MYSQL*
	MYSQL *Get();

};
using SP_SQL = std::shared_ptr<ISQLConnect>;

#endif //ISQLCONNECT_H