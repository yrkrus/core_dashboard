// Класс для получения данных по номеру телефона (регион и оператор)

#ifndef MOBILEOPERATORINFO_H
#define MOBILEOPERATORINFO_H

#include <string>
#include <vector>
#include "interfaces/IHTTPRequest.h"
#include "interfaces/ISQLConnect.h"

static std::string HTTP_REQUEST_INFO = "https://num.voxlink.ru/get/?num=%phone";

enum class ecMobileInfoTable // в какую таблицу будем обновлять
{
    eIVR,
    eHistoryIvr,
    eSmsSending,
    eHistorySmsSending
};

struct MobileInfo
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
using MobileInfoList = std::vector<MobileInfo>;


// список с текущими ошибочными номерами по которым не удается найти инфо 
class MobileInfoError  
{
private:
    MobileInfoList m_list;

public:
    MobileInfoError();
    ~MobileInfoError(); 
    
    bool IsExistPhone(const MobileInfo &_info);
    void Add(const MobileInfo &_info);
};


class MobileOperatorInfo : virtual public IHTTPRequest 
{
public:      

     MobileOperatorInfo() = delete;
     MobileOperatorInfo(ecMobileInfoTable _table, const::std::string &_logName);
    ~MobileOperatorInfo() override;

    virtual bool Execute(); 
    
    SP_SQL  GetSQL();
    SP_Log  GetLog();

private:
    MobileInfoList  m_listPhone;
    SP_SQL		    m_sql;
	SP_Log  		m_log;    

    ecMobileInfoTable m_table;

    MobileInfoError m_listError;

    void CreateListPhone(); // создание списка с телефонами которые будем проверять
    virtual bool GetInfoMobileList(MobileInfoList &_list, std::string &_errorDescription) = 0; // получение списка с телефонами

    bool IsExistList();     // есть ли список

    void FindInfoOperatorAndRegion();    // нахождение региона и оператора 

    std::string GetLinkHttpRequest(const std::string &_phone); // создание http запроса

    void UpdateToBaseInfo(int _id, const MobileInfo &_call); // обновление записи в БД

    virtual bool Get(const std::string &_request, std::string &_responce, std::string &_errorDescription);
};




 #endif // MOBILEOPERATORINFO_H