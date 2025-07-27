#include "ClearingCurrentDay.h"
#include "../Constants.h"

ClearingCurrentDay::ClearingCurrentDay()
	: m_dispether(CONSTANTS::TIMEOUT::CLEARING_CURRENT_DAY)
	, m_day(std::time(nullptr) - YESTERDAY)
{
}

ClearingCurrentDay::~ClearingCurrentDay()
{
	Stop();
}

bool ClearingCurrentDay::CheckNewDay()
{
	return m_day.CheckNewDay();
}

bool ClearingCurrentDay::Execute() 
{
	if (!CheckNewDay()) 
	{
		return false;
	}
			
	printf("Execute clearind Day\n");

	m_ivr.Execute();
	m_logging.Execute();
	m_onHold.Execute();
	m_queue.Execute();
	m_sms.Execute();

	printf("Execute clearing Day is done\n");

	return true;
}

void ClearingCurrentDay::Start()
{
	m_dispether.Start([this]()
		{
			return Execute();
		});
}

void ClearingCurrentDay::Stop()
{
	m_dispether.Stop();
}
