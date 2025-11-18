#include <iostream>
#include <string>
#include <thread>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

#include "interfaces/ISQLConnect.h"
#include "system/Constants.h"
#include "utils/InternalFunction.h"
#include "core/RemoteCommands.h"
#include "core/TCPServer.h"
#include "core/IVR.h"
#include "core/ActiveSip.h"
#include "core/ActiveLisa.h"

#include "clearing_current_day/ClearingCurrentDay.h"
#include "different_checks/CheckInternal.h"

// эти include потом убрать, они нужны для отладки только
#include <stdio.h>
#include <time.h>
#include <chrono>


using namespace utils;
using namespace active_sip;

// Global Score
static SP_IVR ivr                               = nullptr;
static SP_Queue queue                           = nullptr;
static SP_ActiveSession activeSession           = nullptr;
static SP_ActiveLisa    activeLisa              = nullptr;
static SP_Status changeStatus                   = nullptr;
static SP_ClearingCurrentDay clearingCurrentDay = nullptr;  // вставка в таблицы history
static SP_CheckInternal checkInternal           = nullptr;  // внутренние проверки

static std::atomic<bool> g_running(true);

void _daemonize()
{
    pid_t pid = fork();
    if (pid < 0) {
        // не смогли форкнуть
        std::cerr << "First fork failed: " << std::strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }
    if (pid > 0) 
    {
        // родитель выходит
        exit(EXIT_SUCCESS);
    }

    // теперь мы в первом потомке
    // создаём новый сеанс, отрываемся от контролирующего терминала
    if (setsid() < 0) 
    {
        std::cerr << "setsid failed: " << std::strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }

    // Принудительно игнорируем сигналы SIGHUP, SIGCHLD (опционально)
    signal(SIGHUP,  SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // Второй fork, чтобы гарантированно не стать лидером терминального сеанса
    pid = fork();
    if (pid < 0) 
    {
        std::cerr << "Second fork failed: " << std::strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }
    if (pid > 0) 
    {
        // первый потомок уходит
        exit(EXIT_SUCCESS);
    }

    // Устанавливаем рабочую папку
    if (chdir(LOG_DESTINATION) < 0) 
    {
        std::cerr << "chdir failed: " << std::strerror(errno) << "\n";
    }

    // Сбрасываем маску прав
    umask(0);

    // Перенаправляем stdin/stdout/stderr в /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        // не страшно, продолжим
    } else {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
}


static void _Init()
{
    ivr                 = std::make_shared<IVR>();
    queue               = std::make_shared<Queue>();
    activeSession       = std::make_shared<ActiveSession>(queue);
    activeLisa          = std::make_shared<ActiveLisa>();
    changeStatus        = std::make_shared<Status>();
    clearingCurrentDay  = std::make_shared<ClearingCurrentDay>(); // встаква в таблицы history_*
    checkInternal       = std::make_shared<CheckInternal>();  // внутренние проверки  
}

static void _Run()
{
    ivr->Start();
    queue->Start();
    activeSession->Start();
    activeLisa->Start();
    changeStatus->Start();          // изменение статуса оператора
    clearingCurrentDay->Start();    // очистка текущего дня в таблицу history_*
    checkInternal->Start();     
}

static void _Destroy()
{
    ivr->Stop();
    queue->Stop();
    activeSession->Stop();
    activeLisa->Stop();
    changeStatus->Stop();
    clearingCurrentDay->Stop();
    checkInternal->Stop();    
}

static void _sigint_handler(int)
{
    g_running = false;
}

static void _core_info(bool _isDaemonize)
{
    _isDaemonize ? printf("\t\t\t=== RUN IS DAEMON === \n")
                 : printf("\t\t\t=== RUN IS APPLICATION === \n");    
  
    printf("%s\t | base:%s\t | TCP server PORT:%u\n",
            CONSTANTS::VERSION::CORE.c_str(), 
            AUTH::MYSQL::BD.c_str(),
            CONSTANTS::SERVER::PORT);      
    Sleep(3000);
}

int main(int argc, char *argv[])
{  
    bool runAsDaemon = false;
    for (int i = 1; i < argc; ++i) 
    {
        if (std::string(argv[i]) == "-d") 
        {
            runAsDaemon = true;
            break;
        }
    }

    if (runAsDaemon) 
    {
        _core_info(runAsDaemon); 
        _daemonize();
    }    
    
    // bild
    _core_info(runAsDaemon);   

    // Перехватываем Ctrl+C
    std::signal(SIGINT, _sigint_handler);

    HeartbeatServer server(CONSTANTS::SERVER::PORT);
    server.set_on_ping([]()
                       { std::cout << "[PING] got ping\n"; });

    if (!server.start())
    {
        std::cerr << StringFormat("\nServer not started on port %u\n", CONSTANTS::SERVER::PORT);
        return 1;
    }
    std::cout << StringFormat("\nServer is running on port %u. Press Ctrl+C to stop.\n", CONSTANTS::SERVER::PORT);

    _Init();
    _Run();

    while (g_running)
    {
        auto start = std::chrono::steady_clock::now();

        ivr->Parsing();
        queue->Parsing();
        activeSession->Parsing();
        activeLisa->Parsing();

        auto stop = std::chrono::steady_clock::now();
        auto execute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        printf("\n%s time execute code: %ld ms", GetCurrentDateTime().c_str(), execute_ms.count());

        Sleep(10);
    }

    std::cout << "\nServer stopping ...\n";

    _Destroy();

    server.stop();
    std::cout << "\nServer stopped\n";    

    return 0;
};