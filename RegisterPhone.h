#ifndef REGISTER_PHONE_H
#define REGISTER_PHONE_H

#include "interfaces/IHTTPRequest.h"

// static std::string HTTP_REQUEST_REGISTER_PHONE = "https://a2p-sms-https.beeline.ru/proto/http/?gzip=none&user=%user&pass=%pass&action=status&sms_id=%sms_id";
static std::string HTTP_REQUEST_DEREGISTER_PHONE =  "https://%phoneIP/servlet?phonecfg=set[&account.1.enable=0][&account.1.label=''][&account.1.display_name=''][&account.1.auth_name=''][&account.1.user_name=''][&account.1.password='']";

class RegisterPhone : public IHTTPRequest
{
    private:

    
    public:
    RegisterPhone();
    ~RegisterPhone();
};

#endif // REGISTER_PHONE_H