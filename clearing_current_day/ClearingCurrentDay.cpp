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
	m_ivr.Execute();
	m_logging.Execute();
	m_onHold.Execute();
	m_queue.Execute();
	m_sms.Execute();

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
	printf("ClearingCurrentDay stopped!\n");
}


bool Day::CheckNewDay()
	{
		std::tm tm_start = toLocalTm(m_started);
		std::tm tm_current = toLocalTm(m_current);

		// если год изменился — автоматически новый год → новый день
		if (tm_current.tm_year != tm_start.tm_year)
		{
			m_started = m_current;
			return true;
		}
		// если номер дня в году (0–365) изменился
		if (tm_current.tm_yday > tm_start.tm_yday) 
		{
			m_started = m_current;
			return true;
		}			

		return false;
	}