#include <boost/property_tree/ptree.hpp>
#include <map>
#include "SmsInfo.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "utils.h"

using namespace utils;

static std::map<std::string, ecXMLValue> _SMS_XML_VALUE = 
{
    {"output.MESSAGES.MESSAGE.SMSSTC_CODE",ecXMLValue::code},
    {"output.MESSAGES.MESSAGE.SMS_CLOSED",ecXMLValue::close},
    {"output.MESSAGES.MESSAGE.SMS_SENT",ecXMLValue::send},
};

static std::string GetXMlValue(ecXMLValue _value) 
{
    for (const auto &val : _SMS_XML_VALUE) 
    {
       if (val.second == _value) 
       {
            return val.first;
       }
    }  
    
    return std::string{};
};


SMSInfo::SMSInfo()
    : m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::SMS_INFO)
    , m_table(ecSmsInfoTable::eSMS)  
{
    InitAuthSMS();
}

SMSInfo::SMSInfo(ecSmsInfoTable _table)
    : m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::SMS_INFO)
    , m_table(_table)  
{
    InitAuthSMS();
}

SMSInfo::~SMSInfo()
{
}

bool SMSInfo::Execute()
{
    if (!m_auth.IsInit()) 
	{
		m_log.ToFile(ecLogType::eError, "InitAuthSMS() Error!");
	}
	
	// найдем не обработанные смс 
    CreateListSMS();

	if (IsExistList()) 
    {
        FindInfoSMS();
    }

    return true;
}

void SMSInfo::InitAuthSMS()
{
    std::string errorDesciption;	
	
	const std::string query = "select sms_login,sms_pwd from sms_settings where id = '1' ";

	if (!m_sql->Request(query, errorDesciption))
	{
		m_sql->Disconnect();
		errorDesciption = StringFormat("%s\tInitAuth SMS error\t%s",METHOD_NAME,errorDesciption.c_str());
        m_log.ToFile(ecLogType::eError, errorDesciption);
        return;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

    std::string login;
    std::string pwd;

	while ((row = mysql_fetch_row(result)) != NULL)
	{		
		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
				case 0: login   = row[i];	break;
				case 1: pwd     = row[i];   break;				
			}				
		}	
	}

	mysql_free_result(result);
	m_sql->Disconnect();

    if (!login.empty() && !pwd.empty()) 
    {
        m_auth.Init(login,pwd);
    }      
}

void SMSInfo::CreateListSMS()
{
	std::string errorDescription;
    if (!GetInfoSMSList(m_listSMS, errorDescription)) 
    {
        m_log.ToFile(ecLogType::eError, errorDescription);
    }
}

bool SMSInfo::GetInfoSMSList(InfoSMSList &_list, std::string &_errorDescription)
{
	 _list.clear();
    _errorDescription.clear();

    const std::string query = "select id,sms_id,phone,date_time from "+EnumToString<ecSmsInfoTable>(m_table)+" where status not IN ('3') order by date_time DESC";
    	
	if (!m_sql->Request(query, _errorDescription))
	{		
		_errorDescription += METHOD_NAME + StringFormat("query \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}	

	// результат
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		InfoSMS sms;
		
		for (size_t i = 0; i < mysql_num_fields(result); ++i)
		{
			switch (i)
			{
                case 0:	sms.id 		= std::atoi(row[i]); break; 
                case 1:	sms.sms_id 	= string_to_size_t(row[i]); break;
				case 2: sms.phone	= row[i]; break;
                case 3: sms.date	= row[i]; break;               			                            
			}
		}
		_list.push_back(sms);
	}

	mysql_free_result(result);
	m_sql->Disconnect();  

	return true;
}

bool SMSInfo::IsExistList()
{
    return !m_listSMS.empty();
}

void SMSInfo::FindInfoSMS()
{
	for (auto &sms : m_listSMS) 
    {         
        std::string responce;
        std::string errorDescription;
        

        std::string request = GetLinkHttpRequest(sms.sms_id);

        if (!Get(request, responce, errorDescription)) 
        {
            m_table == ecSmsInfoTable::eSMS ? m_log.ToFile(ecLogType::eError,errorDescription)
                                            : m_log.ToPrint(errorDescription);           

            continue;
        }

        // разберем что пришло в ответе       
        boost::property_tree::ptree xml = CreateXML(responce);
        bool smsChecked = true;
        
        using enumType = std::underlying_type_t<ecXMLValue>;
        for (enumType i = 0; i <= static_cast<enumType>(ecXMLValue::send); ++i) 
        {
            if (!ParsingXML(xml, static_cast<ecXMLValue>(i), sms, errorDescription)) 
            {
                errorDescription = StringFormat("phone %s empty XML. %s", sms.phone.c_str(), errorDescription.c_str()); 
                                
                m_table == ecSmsInfoTable::eSMS ? m_log.ToFile(ecLogType::eError, errorDescription)
                                                : m_log.ToPrint(errorDescription);                   

                smsChecked = false;
                break;
            }
        }      

        // проверим данные
        if (!smsChecked) 
        {
            continue;
        }          
        
        UpdateToBaseInfoSMS(sms.id, sms);
        if (m_table == ecSmsInfoTable::eHistorySMS) 
        {
            printf("\nUpdateToBaseInfoSMS: %s\t %s",sms.date.c_str(), sms.phone.c_str());
        }
        
        Sleep(10); 
    }
}

std::string SMSInfo::GetLinkHttpRequest(size_t _smsId)
{
	std::string request = HTTP_REQUEST_SMS;

	// сформируем строку (user)
	ReplaceResponseStatus(request, "%user", m_auth.Login());

	// сформируем строку (pass)
	ReplaceResponseStatus(request, "%pass",  m_auth.Pass());
	
	// сформируем строку (sms_id)
	ReplaceResponseStatus(request, "%sms_id", std::to_string(_smsId));	

	return request;
}


bool SMSInfo::ParsingXML(const boost::property_tree::ptree &_ptree, ecXMLValue _xmlValue, InfoSMS &_sms, std::string &_errorDescription)
{
    _errorDescription.clear();

    if (_ptree.empty()) 
    {
        _errorDescription = StringFormat("%s\txml empty", METHOD_NAME);
        return false;
    }

    std::string valueXML;

    try
    {            
        valueXML = _ptree.get<std::string>(GetXMlValue(_xmlValue));             
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        _errorDescription = StringFormat("%s\txml не найден узел \t%s", METHOD_NAME, e.what());
       return false;
    }

    switch (_xmlValue)
    {
        case ecXMLValue::code: 
        {
            _sms.code = StringToEnum<ecSmsCode>(valueXML); 
            break;      
        }
        case ecXMLValue::close: 
        {
            _sms.close = (valueXML.find("1") != std::string::npos ? true : false);
            break;    
        }
        case ecXMLValue::send: 
        {
            _sms.send = (valueXML.find("1") != std::string::npos ? true : false);
            break;   
        }        
    }

    return true;
}

void SMSInfo::UpdateToBaseInfoSMS(int _id, const InfoSMS &_sms)
{
     std::string errorDescription;
     const std::string query = "update " + EnumToString<ecSmsInfoTable>(m_table) +" set status  = '" + std::to_string(static_cast<int>(_sms.code))
                                                                                 + "' , status_date = '" + GetCurrentDateTime()                                        
                                                                                 + "' where id = '" + std::to_string(_id) + "'";
    
	if (!m_sql->Request(query, errorDescription))
	{
		errorDescription = StringFormat("%s\tquery \t%s",METHOD_NAME, query.c_str());
		m_log.ToFile(ecLogType::eError, errorDescription);

		m_sql->Disconnect();
		return;
	}      

	m_sql->Disconnect();
}

bool SMSInfo::Get(const std::string &_request, std::string &_responce, std::string &_errorDescription)
{
    if (!IHTTPRequest::Get(_request, _responce))
    {
        _errorDescription = StringFormat("%s\t%s", METHOD_NAME, _responce.c_str());
        return false;
    }

    return true;
}

AuthSMS::AuthSMS(std::string _login, std::string _pwd)
    : m_login(_login) 
    , m_pwd(_pwd)
    , m_init(true)
{   
}

AuthSMS::~AuthSMS()
{
}

void AuthSMS::Init(std::string _login, std::string _pwd)
{
    m_login = _login;
    m_pwd = _pwd;

	m_init = true;
}
