#include "SipPhone.h"


static std::string HTTP_REQUEST_REGISTER_PHONE      =  "https://%phoneIP/servlet?phonecfg=set[&account.1.enable=1][&account.1.label=%sip][&account.1.display_name=%sip][&account.1.auth_name=%sip][&account.1.user_name=%sip][&account.1.password=159753]";
static std::string HTTP_REQUEST_DEREGISTER_PHONE    =  "https://%phoneIP/servlet?phonecfg=set[&account.1.enable=0][&account.1.label=''][&account.1.display_name=''][&account.1.auth_name=''][&account.1.user_name=''][&account.1.password='']";
static std::string HTTP_REQUEST_STATUS_PHONE        =  "https://%phoneIP/servlet?phonecfg=get[&accounts=1]";



SipPhone::SipPhone()
{
}

SipPhone::~SipPhone()
{
}
