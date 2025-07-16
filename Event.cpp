#include "Event.h"

Event::Event(std::string _name, bool _isAutoResetEvent, bool _eventIsSet)
	: m_name(_name)
	, m_isAutoResetEvent(_isAutoResetEvent)
	, m_eventIsSet(_eventIsSet)
{
}

Event::~Event()
{
}

void Event::Set()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_eventIsSet = true;
	}

	m_signal.notify_one();
}

void Event::Reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_eventIsSet = false;
}

void Event::Wait()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	while (!m_eventIsSet) 
	{
		m_signal.wait(lock);
	}

	if (m_isAutoResetEvent) 
	{
		m_eventIsSet = false;
	}
}

bool Event::TimedWait(unsigned int _timeout_ms)
{
	return false;
}
