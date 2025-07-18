// TCP Сервер

#ifndef TCPSERVER_H
#define	TCPSERVER_H

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

class HeartbeatServer
{
public:
	using OnPingCallback = std::function<void()>;

	explicit HeartbeatServer(unsigned short port);
	~HeartbeatServer();

	// Запустить сервер (фоновый поток для accept)
	bool start();
	// Остановить сервер (закрывает слушающий сокет и все клиенты)
	void stop();

	// Установить callback, вызываемый при каждом PING
	void set_on_ping(OnPingCallback cb);

private:
	void accept_loop();
	void handle_client(int client_fd);

	unsigned short        port_;
	int                   listen_fd_;
	std::atomic<bool>     is_running_;
	std::thread           accept_thread_;
	OnPingCallback        on_ping_;

	// Для корректного закрытия всех клиентских сокетов
	std::mutex            clients_mtx_;
	std::vector<int>      client_fds_;
};




#endif // TCPSERVER_H