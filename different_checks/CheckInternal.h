#ifndef CHECK_INTERNAL_H
#define CHECK_INTERNAL_H

#include <memory>
#include "Check_TrunkSip.h"
#include "../PotokDispether.h"
#include "../CallInfo.h"
#include "../SmsInfo.h"


class CheckInternal
{
private:
    CheckTrunkSip m_trunkSip;
    CallInfo      m_callInfo;
    SMSInfo       m_smsInfo;  
    IPotokDispether m_dispether;

    bool Execute();

public:
    CheckInternal();
    ~CheckInternal(); 
    
    void Start();  
    void Stop();  
};
using SP_CheckInternal = std::shared_ptr<CheckInternal>; 


#endif //CHECK_INTERNAL_H