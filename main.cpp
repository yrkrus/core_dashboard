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

#include "clearing_current_day/HistorySms.h"

// эти include потом убрать, они нужны для отладки только
#include <stdio.h>
#include <time.h>
#include <chrono>

using namespace utils;
using namespace active_sip;

// Global Score
static SP_IVR ivr                       = nullptr;
static SP_Queue queue                   = nullptr;
static SP_ActiveSession activeSession   = nullptr;
static SP_Status changeStatus           = nullptr;

static std::atomic<bool> g_running(true);

static void _Init() 
{
    ivr             = std::make_shared<IVR>();
    queue           = std::make_shared<Queue>();
    activeSession   = std::make_shared<ActiveSession>(queue);
    changeStatus    = std::make_shared<Status>();
}

static void _Run() 
{
    ivr->Start();
    queue->Start();
    activeSession->Start();

    changeStatus->Start();
}

static void _Destroy()
{
    ivr->Stop();
    queue->Stop();
    activeSession->Stop();

    changeStatus->Stop();
}

static void _sigint_handler(int)
{
    g_running = false;
}



int main(int argc, char *argv[])
{
    
   /* HistorySms history;
    history.Execute();
    
    return 0;*/

    // Перехватываем Ctrl+C
    std::signal(SIGINT, _sigint_handler);

    HeartbeatServer server(CONSTANTS::SERVER::PORT);
    server.set_on_ping([]()
        {
            std::cout << "[PING] got ping\n";
        });
    
    if (!server.start())
    {
        std::cerr << StringFormat("Server not started on port %u\n", CONSTANTS::SERVER::PORT);
        return 1;
    }
    std::cout << StringFormat("Server is running on port %u. Press Ctrl+C to stop.\n",CONSTANTS::SERVER::PORT);
   
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
        std::cout << "\ntime execute code: " << execute_ms.count() << " ms\n";         

        std::this_thread::sleep_for(std::chrono::milliseconds(10));   
    }
       
    std::cout << "\nStopping server...\n";
        
    _Destroy();
        
    server.stop();
    std::cout << "Server stopped.\n";
    return 0;   

   
};