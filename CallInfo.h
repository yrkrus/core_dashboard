#ifndef CALLINFO_H
#define CALLINFO_H

#include <string>
#include <vector>
#include "IHTTPRequest.h"
#include "ISQLConnect.h"

static std::string HTTP_REQUEST_CALL = "https://num.voxlink.ru/get/?num=%phone";

enum class ecCallInfoTable // в какую таблицу будем обновлять
    {
        eIVR,
        eHistoryIvr
    };

struct InfoCall
{
    int         id = 0;
    std::string phone;
    std::string phone_operator;
    std::string region;        

    bool Check() const 
    {
        return ((!phone.empty()) && (!phone_operator.empty()) && (!region.empty()));        
    };
};
using InfoCallList = std::vector<InfoCall>;


// список с текущими ошибочными номерами по которым не удается найти инфо 
class CallInfoError  
{
private:
    InfoCallList m_list;

public:
    CallInfoError();
    ~CallInfoError(); 
    
    bool IsExistCall(const InfoCall &_call);
    void Add(const InfoCall &_call);
};


class CallInfo : public IHTTPRequest 
{
public:      

     CallInfo();
     CallInfo(ecCallInfoTable _table);
    ~CallInfo() override;

    bool Execute();     

private:
    InfoCallList    m_listPhone;
    SP_SQL		    m_sql;
	Log				m_log;    

    ecCallInfoTable m_table;

    CallInfoError m_listError;

    void CreateListPhone(); // создание списка с телефонами которые будем проверять
    bool GetInfoCallList(InfoCallList &_list, std::string &_errorDescription); // получение списка с телефонами

    bool IsExistList();     // есть ли список

    void FindInfoCall();    // нахождение региона и оператора 

    std::string GetLinkHttpRequest(const std::string &_phone); // создание http запроса

    void UpdateToBaseInfoCall(int _id, const InfoCall &_call); // обновление записи в БД

    virtual bool Get(const std::string &_request, std::string &_responce, std::string &_errorDescription);
};


#endif //CALLINFO_H