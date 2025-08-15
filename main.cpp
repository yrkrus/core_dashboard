#include <iostream>
#include <string>
#include <thread>
#include <csignal>

#include "ISQLConnect.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "RemoteCommands.h"
#include "TCPServer.h"
#include "IVR.h"
#include "ActiveSip.h"

#include "clearing_current_day/ClearingCurrentDay.h"
#include "different_checks/CheckInternal.h"
#include "CallInfo.h"

// эти include потом убрать, они нужны для отладки только
#include <stdio.h>
#include <time.h>
#include <chrono>


using namespace utils;
using namespace active_sip;

// Global Score
static SP_IVR ivr = nullptr;
static SP_Queue queue = nullptr;
static SP_ActiveSession activeSession = nullptr;
static SP_Status changeStatus = nullptr;
static SP_ClearingCurrentDay clearingCurrentDay = nullptr; // вставка в таблицы history
static SP_CheckInternal checkInternal = nullptr;    // внутренние проверки

static std::atomic<bool> g_running(true);

static void _Init()
{
    ivr = std::make_shared<IVR>();
    queue = std::make_shared<Queue>();
    activeSession = std::make_shared<ActiveSession>(queue);
    changeStatus = std::make_shared<Status>();
    clearingCurrentDay = std::make_shared<ClearingCurrentDay>(); // встаква в таблицы history_*
    checkInternal = std::make_shared<CheckInternal>();  // внутренние проверки  
}

static void _Run()
{
    ivr->Start();
    queue->Start();
    activeSession->Start();
    changeStatus->Start(); // изменение статуса оператора
    clearingCurrentDay->Start(); // очистка текущего дня в таблицу history_*
    checkInternal->Start();     
}

static void _Destroy()
{
    ivr->Stop();
    queue->Stop();
    activeSession->Stop();
    changeStatus->Stop();
    clearingCurrentDay->Stop();
    checkInternal->Stop();    
}

static void _sigint_handler(int)
{
    g_running = false;
}


int main(int argc, char *argv[])
{   
    // bild
    printf("%s\n\n", CONSTANTS::VERSION::CORE.c_str());
    Sleep(2000);

    // Перехватываем Ctrl+C
    std::signal(SIGINT, _sigint_handler);

    HeartbeatServer server(CONSTANTS::SERVER::PORT);
    server.set_on_ping([]()
                       { std::cout << "[PING] got ping\n"; });

    if (!server.start())
    {
        std::cerr << StringFormat("Server not started on port %u\n", CONSTANTS::SERVER::PORT);
        return 1;
    }
    std::cout << StringFormat("Server is running on port %u. Press Ctrl+C to stop.\n", CONSTANTS::SERVER::PORT);

    std::string body;
    CallInfo call;
    call.Execute();
   


    _Init();
    _Run();

    while (g_running)
    {
        auto start = std::chrono::steady_clock::now();

        ivr->Parsing();
        queue->Parsing();
        activeSession->Parsing();

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