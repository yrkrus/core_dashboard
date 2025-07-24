#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "TCPServer.h"


HeartbeatServer::HeartbeatServer(unsigned short port)
    : port_(port)
    , listen_fd_(-1)
    , is_running_(false)
{
}

HeartbeatServer::~HeartbeatServer()
{
    stop();
}

bool HeartbeatServer::start()
{
    if (is_running_) return true;

    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0)
    {
        perror("socket");
        return false;
    }
    std::cout << "[DEBUG] socket() → " << listen_fd_ << "\n";

    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        ::close(listen_fd_);
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        ::close(listen_fd_);
        return false;
    }
    std::cout << "[DEBUG] bind() success\n";

    if (listen(listen_fd_, SOMAXCONN) < 0)
    {
        perror("listen");
        ::close(listen_fd_);
        return false;
    }
    std::cout << "[DEBUG] listen() → OK " << port_ << "\n";

    is_running_ = true;
    accept_thread_ = std::thread(&HeartbeatServer::accept_loop, this);
    return true;
}

void HeartbeatServer::stop()
{
    if (!is_running_) return;
    is_running_ = false;
    // Закрываем слушающий сокет, чтобы accept вернул ошибку
    ::shutdown(listen_fd_, SHUT_RD);
    ::close(listen_fd_);
    if (accept_thread_.joinable())
        accept_thread_.join();

    // Закрываем все клиентские сокеты
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);
        for (int fd : client_fds_)
        {
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
        }
        client_fds_.clear();
    }
}

void HeartbeatServer::set_on_ping(OnPingCallback cb)
{
    on_ping_ = std::move(cb);
}

void HeartbeatServer::accept_loop()
{
    while (is_running_)
    {
        int client_fd = ::accept(listen_fd_, nullptr, nullptr);
        if (client_fd < 0)
        {
            if (is_running_) perror("accept");
            break;
        }
        {
            std::lock_guard<std::mutex> lk(clients_mtx_);
            client_fds_.push_back(client_fd);
        }
        // Запускаем обработчик клиента в отдельном потоке
        std::thread(&HeartbeatServer::handle_client, this, client_fd)
            .detach();
    }
}

void HeartbeatServer::handle_client(int client_fd)
{
    const size_t BUF_SZ = 1024;
    char buffer[BUF_SZ];
    std::string incoming;

    while (is_running_)
    {
        ssize_t n = ::recv(client_fd, buffer, BUF_SZ, 0);
        if (n <= 0) break; // клиент закрылся или ошибка
        incoming.append(buffer, n);

        // обрабатываем полные строки
        size_t pos;
        while ((pos = incoming.find('\n')) != std::string::npos)
        {
            std::string line = incoming.substr(0, pos);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line == "PING")
            {
                const char *pong = "PONG\n";
                ::send(client_fd, pong, strlen(pong), 0);
                if (on_ping_) on_ping_();
            }
            incoming.erase(0, pos + 1);
        }
    }

    ::close(client_fd);
    // удаляем из списка активных
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);
        auto it = std::find(client_fds_.begin(), client_fds_.end(), client_fd);
        if (it != client_fds_.end())
            client_fds_.erase(it);
    }
}