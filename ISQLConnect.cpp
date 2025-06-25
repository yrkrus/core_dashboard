#include "ISQLConnect.h"
#include "InternalFunction.h"

using namespace INTERNALFUNCTION;

ISQLConnect::ISQLConnect(bool _autoConnect)
    : m_initialized(false),
    m_connected(false)
{
    if (!mysql_init(&m_mysql))
    {
        // не получилось выделить дескриптор MySQL
        throw std::runtime_error("ISQLConnect: mysql_init failed");
    }
    m_initialized = true;

    if (_autoConnect)
    {
        std::string err;
        m_connected = Connect(err);
    }
}

ISQLConnect::~ISQLConnect()
{
    // даже если не удалось подключитьс€, но инициализаци€ была Ч
    // закрываем дескриптор, чтобы не было утечки
    if (m_initialized)
    {
        mysql_close(&m_mysql);
    }
}

bool ISQLConnect::IsConnected() const
{
    return m_connected;
}

bool ISQLConnect::Connect(std::string &_errorDescription)
{
    if (m_connected)
    {
        return true;
    }
    return m_connected = ConnectInternal(_errorDescription);
}

void ISQLConnect::Disconnect()
{
    if (m_initialized && m_connected)
    {
        mysql_close(&m_mysql);
        // если хотим дать возможность повторно Connect() вызывать,
        // надо снова проинициализировать:
        mysql_init(&m_mysql);
        // m_initialized остаЄтс€ true
    }
    m_connected = false;
}

bool ISQLConnect::ConnectInternal(std::string &_errorDescription)
{
    static const char *host     = HOST;
    static const char *login    = LOGIN;
    static const char *pwd      = PWD;
    static const char *bd       = BASE;

    if (!mysql_real_connect(&m_mysql, host, login, pwd, bd, 0, 0, 0))
    {
        _errorDescription = StringFormat("%s Error: can't connect to database: %s",METHOD_NAME, mysql_error(&m_mysql));
        return false;
    }

    mysql_set_character_set(&m_mysql, "utf8");
    return true;
}

bool ISQLConnect::Request(const std::string &_request, std::string &_errorDescription)
{
    _errorDescription.clear();
    if (!m_connected)
    {
        if (!Connect(_errorDescription))
        {
            return false;
        }
    }

    if (mysql_query(&m_mysql, _request.c_str()) != 0)
    {
        _errorDescription = StringFormat("%s mysql_query(%s) error: %s",
                                        METHOD_NAME,
                                        _request.c_str(),
                                        mysql_error(&m_mysql));
        return false;
    }
    return true;
}

bool ISQLConnect::Request(const std::string &_request)
{
    std::string error;
    return Request(_request, error);
}

MYSQL *ISQLConnect::Get()
{
    return &m_mysql;
}