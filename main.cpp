#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include <csignal>

#include "ISQLConnect.h"
#include "CreateFiles.h"
#include "Constants.h"
#include "InternalFunction.h"
#include "SQLRequest.h"
#include "RemoteCommands.h"
#include "HouseKeeping.h"
#include "TCPServer.h"

#include "IVR.h"
#include "ActiveSip.h"

#include "clearing_current_day/HistoryQueue.h"

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

static std::atomic<bool> g_running(true);

static void Init() 
{
    ivr = std::make_shared<IVR>();
    queue = std::make_shared<Queue>();
    activeSession = std::make_shared<active_sip::ActiveSession>(queue);

    changeStatus = std::make_shared<remote::Status>();
}

static void Run() 
{
    ivr->Start();
    queue->Start();
    activeSession->Start();

    changeStatus->Start();
}

static void Destroy()
{
    ivr->Stop();
    queue->Stop();
    activeSession->Stop();

    changeStatus->Stop();
}

static void sigint_handler(int)
{
    g_running = false;
}


enum class Commands
{
  //  help,           // хелп справка
  //  ivr,            // кто в IVR
  //  queue,          // текущая очередь
  //  active_sip_old,     // какие активные sip зарегистрированы в очереди
  // connect_bd,     // убрать потом, это для теста
    start,          // сбор данных в БД   
   // statistics,     // отобразить статистику
  //  remote,         // проверка есть ли удаленные команды на добавление\удаление очереди
    housekeeping,   // внутренния задания на очистку БД таблиц (queue, logging, ivr)
   // test,           // test remove after
};

// получить команду
Commands static getCommand(char *ch) {
    std::string commands = static_cast<std::string> (ch);

 //   if (commands == "help")              return Commands::help;
   // if (commands == "ivr")               return ivr;
   // if (commands == "queue")             return queue;
  //  if (commands == "active_sip")        return active_sip_old;
  //  if (commands == "connect_bd")        return connect_bd;
    if (commands == "start")             return Commands::start;
  //  if (commands == "statistics")        return Commands::statistics;
  //  if (commands == "remote")            return Commands::remote;
    if (commands == "housekeeping")      return Commands::housekeeping;
   // if (commands == "test")              return Commands::test;

  //  return Commands::help;                         // default;
}


//static void thread_Queue_ActiveSIP() {
//    //getQueue();
//    getActiveSip();
//}

// запуск проверки удаленных команд
//static void thread_RemoteCommands() {
//    REMOTE_COMMANDS_old::Remote remote;
//    if (remote.getCountCommand())
//    {
//        remote.startCommand();
//    }
//}

// запуск очистки БД перенос в history
static void thread_HouseKeeping() {
    HOUSEKEEPING::HouseKeeping task;
    
    task.createTask(HOUSEKEEPING::TASKS::TaskQueue);    

    task.createTask(HOUSEKEEPING::TASKS::TaskLogging);   

    task.createTask(HOUSEKEEPING::TASKS::TaskIvr);

   // task.createTask(HOUSEKEEPING::TASKS::TaskOnHold);

   task.createTask(HOUSEKEEPING::TASKS::TaskSmsSending);
    
}


static void collect() {
     //int TIK = 3600;  
    static size_t   all{ 0 };
    static int      min{ 1000 };
    static int      max{ 0 };    

    for (size_t i = 1; /*i <= TIK*/; ++i)
    {          
        showVersionCore(i);

        auto start = std::chrono::steady_clock::now();

        //std::cout << GetCurrentDateTime() + "\t\titeration: \t" << i << "\n\n";       

        std::cout << "\n\n";
        //std::thread th_ivr(getIVR);
       // std::thread th_Queue_ActiveSIP(thread_Queue_ActiveSIP);
       // std::thread th_RemoteCommand(thread_RemoteCommands);
        std::thread th_HouseKeeping(thread_HouseKeeping);

        //if (th_ivr.joinable()) {
        //    th_ivr.join();
        //    //th_ivr.detach();
        //} 
        
        //if (th_Queue_ActiveSIP.joinable()) {
        //    th_Queue_ActiveSIP.join();
        //    //th_Queue_ActiveSIP.detach();
        //}

        //// проверка удаленных команд
        //if (th_RemoteCommand.joinable()) {
        //    th_RemoteCommand.join();
        //}       

        // очистка БД
        if (th_HouseKeeping.joinable()) {
            th_HouseKeeping.join();
        }

        auto stop = std::chrono::steady_clock::now();

        auto execute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout  << "\ntime execute code: " << execute_ms.count() << " ms\n";       

        all += execute_ms.count();

        if (execute_ms.count() < min) { min = static_cast<int>(execute_ms.count()); }
        if (execute_ms.count() > max) { max = static_cast<int>(execute_ms.count()); }

        std::cout  << "avg execute = " << all / i << " ms | min execute = " << min << " ms | max execute = " << max << " ms\n";       

        if (execute_ms.count() < 1000)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - execute_ms.count()));
        }
        else
        {
            // Поток уже заблокирован на 1000 миллисекунд или более, поэтому разблокируем его немедленно.
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
        }

        system("clear");

        if (i >= 10800)
        {
            all = 0;
            i = 1;
            min = 1000;
            max = 0;
        }        
    }
}


int main(int argc, char *argv[])
{
    
    HistoryQueue historyQueue;
    historyQueue.Execute();
    
    return 0;

    // Перехватываем Ctrl+C
    std::signal(SIGINT, sigint_handler);

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
   
   
    {        
        Init();        
        Run();
        
        static int _val = 0;

        while (g_running) 
        {
            auto start = std::chrono::steady_clock::now();

            ivr->Parsing();
            queue->Parsing();
            activeSession->Parsing();          

            auto stop = std::chrono::steady_clock::now();
            auto execute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "\ntime execute code: " << execute_ms.count() << " ms\n";

            if (activeSession->IsExistRawData())
            {
                printf("All RawData activeSession = %u\n ", activeSession->GetRawAllData().size());
            }
            else
            {
                std::cout << "null value " << _val << " (" << execute_ms.count()<<" ms)\n";
            }
         
          //  printf("After Del RawData activeSession = %u\n ", activeSession.GetRawAllData().size());

            ++_val;       
            

            std::this_thread::sleep_for(std::chrono::milliseconds(10));     

        }
       
        std::cout << "\nStopping server...\n";
        
        Destroy();
        
        server.stop();
        std::cout << "Server stopped.\n";
        return 0;   

    }




    Commands ch = getCommand(argv[1]);

    // пошли запросики
    switch (ch)
    {
    /*case(Commands::help): {
            showHelpInfo();
            break;
        }*/
        //case(ivr): {                // запись в БД кто сейчас слушает IVR 
        //    // запрос
        //    getIVR();   
        //    break;
        //}
        //case(queue): {              // запись в БД кто ушел из IVR в очередь
        //    // запрос
        //    getQueue();
        //    break;
        //} 
        //case(active_sip_old): {         // запись в БД кто сейчас с кем разговариваети сколько по времени
        //    // запрос
        //    getActiveSip();
        //    break;
        //}
       /* case(connect_bd): {      
            SQL_REQUEST::SQL base;            
            std::cout << (base.isConnectedBD()) ? "Connect UP\n" : "Connect DOWN!\n";            
                 
            break;
        }     */   
        case(Commands::start):      {
            collect();
            break;
        }    
       /* case(Commands::statistics): {
            stat();
            break;
        }*/
        //case(Commands::remote): {
        //  /* REMOTE_COMMANDS::Remote remote;            
        //    remote.chekNewCommand() ? std::cout << "New command EXIST\n" : std::cout << "New command NO EXIST\n";         
        //    break;*/
        //}
        case(Commands::housekeeping): {
            HOUSEKEEPING::HouseKeeping task;           
            std::cout << "create Task and execute -> TaskQueue\n";
            task.createTask(HOUSEKEEPING::TASKS::TaskQueue);
            std::cout << "done -> TaskQueue\n";
            
            std::cout << "create Task and execute -> TaskLogging\n";
            task.createTask(HOUSEKEEPING::TASKS::TaskLogging); 
            std::cout << "done -> TaskLogging\n";

            std::cout << "create Task and execute -> TaskIvr\n";
            task.createTask(HOUSEKEEPING::TASKS::TaskIvr);
            std::cout << "done -> TaskIvr\n";

            // TODO тут еще onHold, но там баг есть с задвоением данным, так что пока отключен

            std::cout << "create Task and execute -> TaskSms\n";
            task.createTask(HOUSEKEEPING::TASKS::TaskSmsSending);
            std::cout << "done -> TaskSMS\n";

            break;
        }
        //case(Commands::test): {
        //    
        //    SQL_REQUEST::SQL base;
        //    if (base.isConnectedBD())
        //    {
        //       // base.updateOperatorsOnHold(&this);
        //    }

        //    break;
        //}
    }
     return 0;
};