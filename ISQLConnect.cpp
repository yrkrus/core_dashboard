#include "ISQLConnect.h"

ISQLConnect::ISQLConnect() 
	: m_connected(false)
{
	std::string errorDescription;
	// Инициализация структуры MYSQL

	if (!mysql_init(&m_mysql))
	{
		printf("Error: can't create MySQL-descriptor\n");
		return; // Выход, если инициализация не удалась
	}

	m_connected = Connect(m_mysql, errorDescription);
	if (!m_connected) 
	{
		printf("[%s] -> Connection failed: %s\n", METHOD_NAME(), errorDescription);
	}	
}

ISQLConnect::~ISQLConnect()
{
	if (m_connected)
	{
		mysql_close(&m_mysql);
	}
}

bool ISQLConnect::Connect(MYSQL &mysql, std::string &_errorDescription)
{
	const char *host	= HOST;
	const char *login	= LOGIN;
	const char *pwd		= PWD;
	const char *bd		= BASE;
	

	if (!mysql_real_connect(&mysql, host, login, pwd, bd, NULL, NULL, 0))
	{
		printf("%s -> Error: can't connect to database", METHOD_NAME());
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		// showErrorBD(METHOD_NAME() + " -> Error: can't connect to database", &mysql);
		return false;
	};

	mysql_set_character_set(&mysql, "utf8");
	
	return true;
}

bool ISQLConnect::IsConnected() const
{
	return m_connected;
}
