#ifndef CALLINFO_H
#define CALLINFO_H

#include "MobileOperatorInfo.h"

class CallInfo : public MobileOperatorInfo
{
    private:
    ecMobileInfoTable m_table;
    
    // override MobileOperatorInfo
    virtual bool GetInfoMobileList(MobileInfoList &_list, std::string &_errorDescription) override;
    
    
    public:
    CallInfo(ecMobileInfoTable _table, const std::string &_logName);
    ~CallInfo() override;

    virtual bool Execute() override; 
};


#endif //CALLINFO_H