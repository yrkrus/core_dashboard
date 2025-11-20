#include "IHTTPRequest.h"
#include "../system/Constants.h"
#include "../utils/InternalFunction.h"

using namespace utils; 

size_t IHTTPRequest::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total);
    
    return total;
}

IHTTPRequest::IHTTPRequest()
    : m_curl(nullptr)
    , m_initialized(false)
    , m_log(std::make_shared<Log>(CONSTANTS::LOG::HTTP_REQUEST))
{
    // Глобальная инициализация libcurl
    if (curl_global_init(CURL_GLOBAL_DEFAULT) == 0)
    {
        m_curl = curl_easy_init();
        m_initialized = (m_curl != nullptr);
    } else 
    {
        m_log->ToFile(ecLogType::eError, "Ошибка curl_global_init()");
    }     
}

IHTTPRequest::~IHTTPRequest()
{
    if (m_curl) 
    {
        curl_easy_cleanup(m_curl);
    }            

    // Очистка глобальных ресурсов
    curl_global_cleanup();
}

bool IHTTPRequest::Get(const std::string &_url)
{
    std::string response;

    return Get(_url, response);
}

bool IHTTPRequest::Get(const std::string &_url, std::string &_outBody)
{
    if (!m_initialized)
    {        
        _outBody = StringFormat("%s\tCURL not init", METHOD_NAME); 
        m_log->ToFile(ecLogType::eError, _outBody);        
        return false;
    }

    _outBody.clear();

    // Устанавливаем URL
    curl_easy_setopt(m_curl, CURLOPT_URL, _url.c_str());

    // Следовать редиректам
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Колбэк для записи ответа
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA,    &_outBody);

    // Таймауты (по желанию)
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT,        30L);

    // Выполняем запрос
    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK)
    {        
        _outBody =  StringFormat("%s\tGet\t %s \t curl_easy_perform() error: %s", METHOD_NAME, _url.c_str(), curl_easy_strerror(res)); 
        m_log->ToFile(ecLogType::eError, _outBody);        
        return false;
    }

    // Проверяем HTTP-код
    long http_code = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300)
    {        
        _outBody = StringFormat("%s\tGet \t %s \tCode %u", METHOD_NAME, _url.c_str(), http_code);
        m_log->ToFile(ecLogType::eError, _outBody);
        return false;
    }

    return true;
}
