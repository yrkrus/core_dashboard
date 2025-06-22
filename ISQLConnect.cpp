#include "ISQLConnect.h"
#include "InternalFunction.h"

using namespace INTERNALFUNCTION;

ISQLConnect::ISQLConnect(bool _connected)
	: m_connected(false)
{
	std::string errorDescription;

	// Инициализация структуры MYSQL
	if (!mysql_init(&m_mysql))
	{
		printf("Error: can't create MySQL-descriptor\n");
	}

	if (_connected)
	{
		m_connected = Connect(m_mysql, errorDescription);
	}
}

ISQLConnect::~ISQLConnect()
{
	if (&m_mysql)
	{
		mysql_close(&m_mysql);		
	}

	m_connected = false;
}

bool ISQLConnect::Connect(MYSQL &mysql, std::string &_errorDescription)
{
	const char *host	= HOST;
	const char *login	= LOGIN;
	const char *pwd		= PWD;
	const char *bd		= BASE;
	

	if (!mysql_real_connect(&mysql, host, login, pwd, bd, NULL, NULL, 0))
	{
		_errorDescription = StringFormat("%s Error: can't connect to database", METHOD_NAME);		
		return false;
	};

	mysql_set_character_set(&mysql, "utf8");
	
	return true;
}


bool ISQLConnect::IsConnected() const
{
	return m_connected;
}

bool ISQLConnect::Connect(std::string &_errorDescription)
{	
	// Инициализация структуры MYSQL
	if (!mysql_init(&m_mysql))
	{
		_errorDescription = StringFormat("%s Error: can't create MySQL-descriptor\n", METHOD_NAME);
		printf("%s",_errorDescription.c_str());
	}		
	m_connected = Connect(m_mysql, _errorDescription);
	return m_connected;
}

void ISQLConnect::Disconnect()
{
	mysql_close(&m_mysql);
	m_connected = false;
}

bool ISQLConnect::Request(const std::string &_request, std::string &_errorDescription)
{
	_errorDescription = "";

	if (!IsConnected())
	{
		if (!Connect(_errorDescription))
		{
			return false;
		}
	}
	
	if (mysql_query(&m_mysql, _request.c_str()) != 0)
	{
		// ошибка 
		_errorDescription = StringFormat("%s mysql_request (%s) error %s", METHOD_NAME, _request, mysql_error(&m_mysql));		
		return false;
	}

	return true;
}

bool ISQLConnect::Request(const std::string &_request)
{	
	std::string error;
	if (!IsConnected())
	{
		if (!Connect(error))
		{
			return false;
		}
	}

	if (mysql_query(&m_mysql, _request.c_str()) != 0)
	{
		// ошибка 
		error = StringFormat("%s mysql_request (%s) error %s", METHOD_NAME, _request, mysql_error(&m_mysql));
		return false;
	}

	return true;
}

MYSQL *ISQLConnect::Get()
{
	return &m_mysql;
}
