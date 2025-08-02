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
	std::time_t now = m_current;
    std::tm currentTm = toLocalTm(now);
    std::tm startTm = toLocalTm(m_started);

    // Проверка года.  Это самый быстрый способ определить новый день.
    if (currentTm.tm_year != startTm.tm_year) 
	{
        m_started = now;
        return true;
    }

    // Проверка дня года.  Эффективнее, чем проверка часов/минут/секунд.
    if (currentTm.tm_yday > startTm.tm_yday) 
	{
        m_started = now;
        return true;
    }

    // Теперь проверяем часы, минуты и секунды только если день тот же.
    if (currentTm.tm_hour > startTm.tm_hour ||
        (currentTm.tm_hour == startTm.tm_hour && currentTm.tm_min > startTm.tm_min) ||
        (currentTm.tm_hour == startTm.tm_hour && currentTm.tm_min == startTm.tm_min && currentTm.tm_sec > startTm.tm_sec)) 
	{
        m_started = now;
        return true;
    }

    return false;
}