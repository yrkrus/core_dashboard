#include "PotokDispether.h"

IPotokDispether::IPotokDispether(unsigned int _timer)
	: m_timer(_timer)
	, m_running(false)
{
}

IPotokDispether::~IPotokDispether()
{
	Stop();
}

void IPotokDispether::Start(std::function<bool()> func)
{
	m_running = true;
	m_thread = std::thread(&IPotokDispether::Run, this, func); // запуск потока
}

void IPotokDispether::Stop()
{
	m_running = false;
	if (m_thread.joinable()) 
	{
		m_thread.join(); // ожидаем завершения потока
	}
}

void IPotokDispether::Run(std::function<bool()> func)
{
	while (m_running) 
	{
		func(); // запускаем переданную функцию
		std::this_thread::sleep_for(std::chrono::milliseconds(m_timer));
	}
}

