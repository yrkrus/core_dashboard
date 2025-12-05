#ifndef CLEARING_CURRENT_DAY_H
#define CLEARING_CURRENT_DAY_H

#include "HistoryIvr.h"
#include "HistoryLogging.h"
#include "HistoryOnHold.h"
#include "HistoryQueue.h"
#include "HistoryQueueLisa.h"
#include "HistorySms.h"
#include "../interfaces/IPotokDispether.h"

#define FULL_DAY 86400

struct Day
{
	std::time_t m_beginDay;	// текущее начало дня отсчет с yyyy-mm-dd 00:00:00	
	bool m_firstRun; // флаг того что первый раз запустили и нужно провериться

	Day(const std::time_t &_beginDay)
		: m_beginDay(_beginDay)		
		, m_firstRun(true) 
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

	bool CheckNewDay();
};

class ClearingCurrentDay
{
private:
	HistoryIvr 			m_ivr;
	HistoryLogging 		m_logging;
	HistoryOnHold 		m_onHold;
	HistoryQueue 		m_queue;
	HistoryQueueLisa 	m_queueLisa;
	HistorySms 			m_sms;
	IPotokDispether 	m_dispether;
	
	Day 				m_day;
	bool 				is_error;	// флаг того что была ли ошибка при выполнении

	bool CheckNewDay(); // проверка дня что нужно запустить очистку
	bool Execute();

public:
	ClearingCurrentDay();
	~ClearingCurrentDay();

	void Start();
	void Stop();
};
using SP_ClearingCurrentDay = std::shared_ptr<ClearingCurrentDay>;

#endif // CLEARING_CURRENT_DAY_H