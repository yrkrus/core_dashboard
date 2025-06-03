// *************************************************************
//			виртуальный класс для SQL запросов
// *************************************************************

#include <mysql/mysql.h>
#include "DEBUG.h"
#include <string>
#include "AUTH.h"



#ifndef ISQLCONNECT_H
#define ISQLCONNECT_H

#define HOST cHOST.c_str();
#define BASE cBD.c_str();
#define LOGIN cLOGIN.c_str();
#define PWD cPASSWORD.c_str();


class ISQLConnect 
{

public:
	ISQLConnect();
	virtual ~ISQLConnect();

	bool IsConnected() const;

private:
	MYSQL m_mysql;	
	bool m_connected;

	// подключаемся к БД MySQL
	bool Connect(MYSQL &mysql, std::string &_errorDescription);

	virtual void Update() = 0;
	virtual void Delete() = 0;
	virtual void Select() = 0;
	virtual void Insert() = 0;
};


#endif //ISQLCONNECT_H