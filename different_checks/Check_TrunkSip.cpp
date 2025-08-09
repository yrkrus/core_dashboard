#include <sstream>
#include <regex>
#include "Check_TrunkSip.h"
#include "../Constants.h"
#include "../InternalFunction.h"
#include "../utils.h"

using namespace utils;

void CheckTrunkSip::Start()
{
    std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(TRUNK_SIP_REQUEST, error);
		};

	m_dispether.Start(func);
}

void CheckTrunkSip::Stop()
{
    m_dispether.Stop();
	printf("CheckTrunkSip stopped!\n");
}

void CheckTrunkSip::Parsing()
{
    m_listTrunk.clear(); // обнулим текущий список	
	
	std::string rawLines = GetRawLastData();
	if (rawLines.empty()) 
	{
		return;
	}	

	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))	
	{
		Trunk trunk;    
		if (CreateTrunk(line, trunk)) 
		{
			m_listTrunk.push_back(trunk);
		}
		else 		
		{
			// удаляем из сырых данных
			DeleteRawLastData();
			break;
		}
	}

	// что то есть нужно теперь в БД запихнуть
	if (IsExistListTrunk())
	{		
	   for (const auto trunk : m_listTrunk) 
       {
            std::string error;
            if (!UpdateTrunkStatus(trunk, error)) 
            {
                m_log.ToFile(ecLogType::eError, error);
            }
       }      

		// удаляем из сырых данных
		DeleteRawLastData();
	}
}

bool CheckTrunkSip::CreateTrunk(const std::string &_lines, Trunk &_trunk)
{
     // \S+ — непробельные подряд, \s+ — один или более пробельных
    // (.+)$  — «всё остальное» до конца строки
    static const std::regex re(
        R"(^(\S+)\s+\S+\s+(\S+)\s+\S+\s+(\S+)\s+(.+)$)"
        //    ^      tok[1]    tok[2]      tok[3]       tok[4]
    );

    std::smatch math;
    if (!std::regex_match(_lines, math, re))
        return false;

    _trunk.host      = math[1].str();  
    _trunk.user_name = math[2].str();  
    _trunk.state     = StringToEnum<ecTrunkState>(math[3].str());  
    _trunk.reg_time  = math[4].str();  

    return true;    
}

bool CheckTrunkSip::IsExistListTrunk() const
{
    return !m_listTrunk.empty();
}

bool CheckTrunkSip::UpdateTrunkStatus(const Trunk &_trunk, std::string &_errorDescription)
{
    _errorDescription.clear();	   
    
    const std::string query = "update sip_trunks set state = '" + EnumToString<ecTrunkState>(_trunk.state) 
                                            + "', date_time_update = '" + GetCurrentDateTime()                                           
                                            + "' where username = '" + _trunk.user_name 
                                            +"' and is_monitoring = '1'";
                                            

	if (!m_sql->Request(query, _errorDescription))
	{
		_errorDescription += METHOD_NAME + StringFormat("\tquery \t%s", query.c_str());
		m_log.ToFile(ecLogType::eError, _errorDescription);

		m_sql->Disconnect();
		return false;
	}

	m_sql->Disconnect();

    return true;
}

CheckTrunkSip::CheckTrunkSip()
    : IAsteriskData("CheckTrunkSip",CONSTANTS::TIMEOUT::CHECK_TRUNK_SIP)
    , m_log(CONSTANTS::LOG::CHECK_TRUNK_SIP)
    , is_running(false)
    , m_sql(std::make_shared<ISQLConnect>(false))
{
}

CheckTrunkSip::~CheckTrunkSip()
{
    Stop();
}

bool CheckTrunkSip::Execute()
{
    if (!is_running) 
    {
        Start();
        is_running = true;
    }
    
    Parsing();

    return true;
}
