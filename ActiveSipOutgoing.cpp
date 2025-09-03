#include "ActiveSipOutgoing.h"
#include "Constants.h"

active_sip_outgoing::ActiveSessionOutgoing::ActiveSessionOutgoing()
    : IAsteriskData("ActiveSessionOutgoing", CONSTANTS::TIMEOUT::ACTIVE_SESSION_OUTGOING)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::ACTIVE_SESSION_OUTGOING)
{
}

active_sip_outgoing::ActiveSessionOutgoing::~ActiveSessionOutgoing()
{
}

void active_sip_outgoing::ActiveSessionOutgoing::Start()
{
}

void active_sip_outgoing::ActiveSessionOutgoing::Stop()
{
}

void active_sip_outgoing::ActiveSessionOutgoing::Parsing()
{
}
