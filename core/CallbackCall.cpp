#include "CallbackCall.h"
#include "../system/Constants.h"


CallbackCall::CallbackCall()
    : m_sql(std::make_shared<ISQLConnect>(false))
	, m_dispether("callbackcall",CONSTANTS::TIMEOUT::CALLBACKCALL)
	, m_log(std::make_shared<Log>(CONSTANTS::LOG::CALLBACKCALL))
{
}

CallbackCall::~CallbackCall()
{
}

void CallbackCall::Start()
{
    m_dispether.Start([this]()
                      { return Execute(); });
}

void CallbackCall::Stop()
{
    m_dispether.Stop();
	printf("CallbackCall stopped!\n");
}

bool CallbackCall::Execute()
{
    return false;
}
