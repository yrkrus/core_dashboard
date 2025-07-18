// *************************************************************
//			виртуальный класс для SQL запросов
// *************************************************************

#ifndef ISQLCONNECT_H
#define ISQLCONNECT_H

#include <mysql/mysql.h>
#include <string>
#include <memory>
#include "DEBUG.h"
#include "AUTH.h"

#define HOST cHOST.c_str();
#define BASE cBD.c_str();
#define LOGIN cLOGIN.c_str();
#define PWD cPASSWORD.c_str();


class ISQLConnect 
{

public:
	ISQLConnect(bool _autoConnect = false);

	virtual ~ISQLConnect();	
	bool IsConnected() const;

	bool Connect(std::string &_errorDescription);
	void Disconnect();

	bool Request(const std::string &_request, std::string &_errorDescription);
	bool Request(const std::string &_request);

	// доступ к сырым MYSQL*
	MYSQL *Get();

private:
	MYSQL	m_mysql;	
	bool	m_connected;
	bool	m_initialized;  // был ли вызван mysql_init

	// подключаемся к БД MySQL
	bool ConnectInternal(std::string &_errorDescription);
};

using SP_SQL = std::shared_ptr<ISQLConnect>;

#endif //ISQLCONNECT_H