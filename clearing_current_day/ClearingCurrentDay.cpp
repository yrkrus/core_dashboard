#include "ClearingCurrentDay.h"
#include "../Constants.h"
#include "../InternalFunction.h"

using namespace utils;

ClearingCurrentDay::ClearingCurrentDay()
	: m_dispether("ClearingCurrentDay",CONSTANTS::TIMEOUT::CLEARING_CURRENT_DAY)
	, m_day(string_to_unix_timeshtamp(GetCurrentStartDay()))
	, is_error(false)
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
	// if (!is_error) 
	// {
	// 	if (!CheckNewDay()) 
	// 	{
	// 		return false;
	// 	}
	// }

	if ((!is_error) && (!CheckNewDay()))
	{
		return false;
	}	

	// если в каком либо методе false считаем это за ошибку
	is_error = !( m_ivr.Execute()
				&& m_logging.Execute()
				&& m_onHold.Execute()
				&& m_queue.Execute()
				&& m_sms.Execute());	

	return true;
}

void ClearingCurrentDay::Start()
{
	m_dispether.Start([this]()
					  { return Execute(); });
}

void ClearingCurrentDay::Stop()
{
	m_dispether.Stop();
	printf("ClearingCurrentDay stopped!\n");
}

bool Day::CheckNewDay()
{
	// первая проверка при первом запуске
	if (m_firstRun)
	{
		m_firstRun = false;
		return true;
	}	
	
	time_t current = std::time(nullptr);

	if (current - m_beginDay > FULL_DAY) 
	{
		m_beginDay = string_to_unix_timeshtamp(GetCurrentStartDay());
		return true; 
	}  

	return false;
}