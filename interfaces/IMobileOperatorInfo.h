// Класс для получения данных по номеру телефона (регион и оператор)
#ifndef IMOBILEOPERATORINFO_H
#define	IMOBILEOPERATORINFO_H

#include <string>
#include <vector>
#include "IHTTPRequest.h"
#include "ISQLConnect.h"



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


class IMobileOperatorInfo : public IHTTPRequest
{
    public:
    IMobileOperatorInfo() = delete;
    IMobileOperatorInfo(const std::string &_logName);
    virtual ~IMobileOperatorInfo() override;

    virtual bool Execute();

    private:
    InfoCallList    m_list;
    Log             m_log;
    SP_SQL		    m_sql;

};





#endif // IMOBILEOPERATORINFO_H