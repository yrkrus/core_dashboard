// Базовый класс для многопоточности

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

#ifndef POTOK_DISPETHER_H
#define POTOK_DISPETHER_H

class IPotokDispether 
{
private:
	unsigned int m_timer;					// время периодичности с которой будет запускаться поток
	std::atomic<bool> m_running;			// флаг для управления потоком
	std::thread m_thread;					// сам поток

	void Run(std::function<bool()> func);	// процесс выполнения

public:
	IPotokDispether() = delete;				// конструктор по умолчанию не нужен, т.к. нужно обязательно timeout передавать

	IPotokDispether(unsigned int _timer);
	virtual ~IPotokDispether();

	void Start(std::function<bool()> func);	// запуск потока
	void Stop();							// остановка потока
};

#endif // POTOK_DISPETHER_H