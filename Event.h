#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>

#ifndef EVENT_H 
#define EVENT_H

class Event 
{
private:
	std::string				m_name;
	bool					m_isAutoResetEvent;
	std::mutex				m_mutex;
	std::condition_variable m_signal;
	bool					m_eventIsSet;

public:
	Event(std::string _name = "Event", bool _isAutoResetEvent = true, bool _eventIsSet = false);
	~Event();

	void Set();

	void Reset();

	void Wait();

	bool TimedWait(unsigned int _timeout_ms = 100);


};
using SP_Event = std::shared_ptr<Event>;



#endif // EVENT_H