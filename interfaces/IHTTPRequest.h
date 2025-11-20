#ifndef IHTTP_REQUEST_H
#define IHTTP_REQUEST_H

#include <curl/curl.h>
#include <string>
#include "../system/Log.h"

class IHTTPRequest
{
private:
    CURL    *m_curl;
    bool    m_initialized;
    SP_Log  m_log;

    // Колбэк для записи ответа в std::string
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    IHTTPRequest();
    virtual ~IHTTPRequest();

    // Выполнить GET, вернуть true при HTTP 2xx
    virtual bool Get(const std::string &_url);

    // Выполнить GET и получить тело ответа
    virtual bool Get(const std::string &_url, std::string &_outBody);
};


#endif //IHTTP_REQUEST_H