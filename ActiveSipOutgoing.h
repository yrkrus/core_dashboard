// Активные звонки ИСХОДЯЩИЕ!

#ifndef ACTIVESIPOUTGOING_H
#define ACTIVESIPOUTGOING_H

#include <string>
#include <vector>
#include "interfaces/IAsteriskData.h"
#include "interfaces/ISQLConnect.h"
#include "system/Log.h"

namespace active_sip_outgoing
{ 
    class ActiveSessionOutgoing : public IAsteriskData	// класс в котором будет жить данные по активным сессиям операторов 
    {
    public:
        ActiveSessionOutgoing();
        ~ActiveSessionOutgoing() override;

         // override IAsteriskData 
        void Start() override;
        void Stop() override;
        void Parsing() override;				// разбор сырых данных
        
    private:			
        // OperatorList	m_listOperators;	
        // ActiveTalkCallList	m_listCall;			
        SP_SQL			m_sql;		
        Log				m_log;
        // IFile			m_rawDataTalkCall;

        
    };
}
using SP_ActiveSessionOutgoing = std::shared_ptr<active_sip_outgoing::ActiveSessionOutgoing>;


#endif // ACTIVESIPOUTGOING_H