#ifndef CALLINFO_H
#define CALLINFO_H

#include <string>
#include <vector>
#include "IHTTPRequest.h"
#include "ISQLConnect.h"

static std::string HTTP_REQUEST = "https://num.voxlink.ru/get/?num=%phone";

class CallInfo : public IHTTPRequest 
{
public:
    struct Info
    {
        std::string phone;
        std::string phone_operator;
        std::string region;
    };
    using InfoCallList = std::vector<Info>;

     CallInfo();
    ~CallInfo() override;

    void Execute();     

private:
    InfoCallList    m_listPhone;
    SP_SQL		    m_sql;
	Log				m_log;

    void CreateListPhone(); // создание списка с телефонами которые будем проверять
    bool GetInfoCallList(InfoCallList &_list, std::string &_errorDescription); // получение списка с телефонами

    bool IsExistList();     // есть ли список

    void FindInfoCall();    // нахождение региона и оператора 

    std::string GetLinkHttpRequest(const std::string &_phone); // создание http запроса

    virtual bool Get(const std::string &_request, std::string &_responce, std::string &_errorDescription);
};

#endif //CALLINFO_H