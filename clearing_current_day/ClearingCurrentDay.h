#ifndef CLEARING_CURRENT_DAY_H
#define CLEARING_CURRENT_DAY_H

#include "HistoryIvr.h"
#include "HistoryLogging.h"
#include "HistoryOnHold.h"
#include "HistoryQueue.h"
#include "HistorySms.h"
#include "../PotokDispether.h"

#define YESTERDAY 86400

struct Day 
{
	std::time_t		m_started;
	std::time_t		m_current;

	Day(const std::time_t &_started)
		: m_started(_started)
		, m_current(std::time(nullptr))
	{		
	};

	static std::tm toLocalTm(std::time_t t)
	{
		std::tm tm{};
#if defined(_MSC_VER)
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif
		return tm;
	}

	inline bool CheckNewDay()
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
};

class ClearingCurrentDay 
{
private:
	HistoryIvr		m_ivr;
	HistoryLogging	m_logging;
	HistoryOnHold	m_onHold;
	HistoryQueue	m_queue;
	HistorySms		m_sms;
	IPotokDispether	m_dispether;

	Day				m_day;
	
	bool CheckNewDay();			// проверка дня что нужно запустить очистку 
	bool Execute();

public:
	ClearingCurrentDay();
	~ClearingCurrentDay();

	void Start();
	void Stop();

};
using SP_ClearingCurrentDay = std::shared_ptr<ClearingCurrentDay>;

#endif // CLEARING_CURRENT_DAY_H