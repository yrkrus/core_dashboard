// Базовый класс для многопоточности

#ifndef POTOK_DISPETHER_H
#define POTOK_DISPETHER_H

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <condition_variable>
#include <mutex>

class IPotokDispether
{
private:
	std::string m_name;		 	// текущее имя потока
	unsigned int m_timer;		 // время периодичности с которой будет запускаться поток
	std::atomic<bool> m_running; // флаг для управления потоком
	std::thread m_thread;		 // сам поток
	std::mutex m_mutex;
	std::condition_variable m_cv;

	void Run(std::function<bool()> func); // процесс выполнения

public:
	IPotokDispether() = delete; // конструктор по умолчанию не нужен, т.к. нужно обязательно timeout передавать

	IPotokDispether(const std::string &_name, unsigned int _timer);
	virtual ~IPotokDispether();

	void Start(std::function<bool()> func); // запуск потока
	void Stop();							// остановка потока
};

#endif // POTOK_DISPETHER_H