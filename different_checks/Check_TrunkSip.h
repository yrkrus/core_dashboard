#ifndef ICHECK_TRUNK_SIP_H
#define ICHECK_TRUNK_SIP_H

#include <string>
#include <vector>

#include "ICheck.h"
#include "../system/Log.h"
#include "../interfaces/IAsteriskData.h"
#include "../interfaces/ISQLConnect.h"

static std::string TRUNK_SIP_COMMANDS_EXT1	= "Host";					// пропуск этой записи
static std::string TRUNK_SIP_REQUEST = "asterisk -rx \"sip show registry\" | grep -v \""+TRUNK_SIP_COMMANDS_EXT1+ "\" ";

enum class ecTrunkState
{
    eUnknown     = -1,
    eRegistered  = 0,
    eRequest     = 1,

};

class CheckTrunkSip : public ICheck
                    , public IAsteriskData  
{
public:
    CheckTrunkSip();
    virtual ~CheckTrunkSip() override;
    
    // ICheck
    virtual bool Execute() override;

    struct Trunk
    {
        std::string host;              // текущий host к которому подключение идет
        std::string user_name;          
        ecTrunkState state = ecTrunkState::eUnknown;
        std::string reg_time;          // время последнего коннекта        
    };

private:    
    Log                 m_log;
    bool                is_running;
    SP_SQL				m_sql;
    std::vector<Trunk>  m_listTrunk;


    // IAsteriskData      
    virtual void Start() override;	
	virtual void Stop() override;  
	virtual void Parsing() override;
   
    bool CreateTrunk(const std::string&, Trunk&);
    bool IsExistListTrunk() const;
    bool UpdateTrunkStatus(const Trunk &_trunk, std::string &_errorDescription);

};

using SP_CheckTrunkSip = std::shared_ptr<CheckTrunkSip>;

#endif //ICHECK_TRUNK_SIP_H