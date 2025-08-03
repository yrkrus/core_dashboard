#include "PotokDispether.h"

IPotokDispether::IPotokDispether(const std::string &_name,unsigned int _timer)
	: m_name(_name)
	, m_timer(_timer)
	, m_running(false)
{
}

IPotokDispether::~IPotokDispether()
{
	Stop();
}

void IPotokDispether::Start(std::function<bool()> func)
{
	// Гарантируем, что мы стартуем поток только один раз:
	bool expected = false;
	if (!m_running.compare_exchange_strong(expected, true))
	{
		return;
	}
	// Передаём func в поток
	m_thread = std::thread(&IPotokDispether::Run, this, std::move(func));
}

void IPotokDispether::Stop()
{
	m_running = false;

	// Разбудить wait_for, если он сейчас спит
	m_cv.notify_one();
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void IPotokDispether::Run(std::function<bool()> func)
{
	while (m_running.load())
	{
		func();

		std::unique_lock<std::mutex> lock(m_mutex);
		// Ждём или уведомления Stop(), или истечения таймаута
		m_cv.wait_for(lock,
					  std::chrono::milliseconds(m_timer),
					  [this] { return !m_running.load(); }
		);
	}
}

