#ifndef SIP_PHONE_H
#define SIP_PHONE_H

#include "../interfaces/IHTTPRequest.h"

class SipPhone : public IHTTPRequest
{
    private:

    
    public:
    SipPhone();
    virtual ~SipPhone() override;
};

#endif // SIP_PHONE_H