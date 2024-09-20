#include <iostream>
#include <string>
#include <unistd.h>
#include "Constants.h"
#include "InternalFunction.h"
#include "SQLRequest.h"
#include "RemoteCommands.h"
#include "HouseKeeping.h"
#include <thread>

// эти include потом убрать, они нужны для отладки только
#include <stdio.h>
#include <time.h>
#include <chrono>


enum Commands
{
    help,           // хелп справка
    ivr,            // кто в IVR
    queue,          // текущая очередь
    active_sip,     // какие активные sip зарегистрированы в очереди
    connect_bd,     // убрать потом, это для теста
    start,          // сбор данных в БД   
    statistics,     // отобразить статистику
    remote,         // проверка есть ли удаленные команды на добавление\удаление очереди
    housekeeping,   // внутренния задания на очистку БД таблиц (queue, logging, ivr)
    test,           // test remove after
};

// получить команду
Commands getCommand(char *ch) {
    std::string commands = static_cast<std::string> (ch);

    if (commands == "help")              return help;
    if (commands == "ivr")               return ivr;
    if (commands == "queue")             return queue;
    if (commands == "active_sip")        return active_sip;
    if (commands == "connect_bd")        return connect_bd;
    if (commands == "start")             return start;
    if (commands == "statistics")        return statistics;
    if (commands == "remote")            return remote;
    if (commands == "housekeeping")      return housekeeping;
    if (commands == "test")              return test;

    return help;                         // default;
}


void thread_Queue_ActiveSIP() {
    getQueue();
    getActiveSip();
}

// запуск проверки удаленных команд
void thread_RemoteCommands() {
    REMOTE_COMMANDS::Remote remote;
    if (remote.getCountCommand())
    {
        remote.startCommand();
    }
}

void stat() {
    int TIK = 6000;
    // int avg{0};
    size_t all{ 0 };
    int min{ 1000 };
    int max{ 0 };

    for (size_t i = 1; /*i <= TIK*/; ++i)
    {

        showVersionCore();
        
        auto start = std::chrono::steady_clock::now();

        std::cout << getCurrentDateTime() + "\t\titeration: \t" << i << "\n\n";

        getStatistics();

        
        // проверка
        REMOTE_COMMANDS::Remote remote;
        if (remote.getCountCommand())
        {
            remote.startCommand();
        }
        

        auto stop = std::chrono::steady_clock::now();

        auto execute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << "\ntime execute code: " << execute_ms.count() << " ms\n";
        all += execute_ms.count();

        if (execute_ms.count() < min) { min = execute_ms.count(); }
        if (execute_ms.count() > max) { max = execute_ms.count(); }

        std::cout << "avg execute = " << all / i << " ms | min execute = " << min << " ms | max execute = " << max << " ms\n";
               
        sleep(1);
        system("clear");

        if (i >= 10800)
        {
            all = 0;
            i = 1;
            int min = 1000;
            int max = 0;
        }
    }
}

void collect() {
    int TIK = 6000;
   // int avg{0};
    size_t all{ 0 };
    int min{ 1000 };
    int max{ 0 };    

    for (size_t i = 1; /*i <= TIK*/; ++i)
    {          

        showVersionCore();

        auto start = std::chrono::steady_clock::now();

        std::cout << getCurrentDateTime() + "\t\titeration: \t" << i << "\n\n";

        std::thread th_ivr(getIVR);
        std::thread th_Queue_ActiveSIP(thread_Queue_ActiveSIP);
        std::thread th_RemoteCommand(thread_RemoteCommands);

        if (th_ivr.joinable()) {
            th_ivr.join();
            //th_ivr.detach();
        } 
        
        if (th_Queue_ActiveSIP.joinable()) {
            th_Queue_ActiveSIP.join();
            //th_Queue_ActiveSIP.detach();
        }

        // проверка удаленных команд
        if (th_RemoteCommand.joinable()) {
            th_RemoteCommand.join();
        }
        

        //getIVR();
        //getQueue();
        //getActiveSip();          

        auto stop = std::chrono::steady_clock::now();

        auto execute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout  << "\ntime execute code: " << execute_ms.count() << " ms\n";
        all += execute_ms.count();

        if (execute_ms.count() < min) { min = execute_ms.count(); }
        if (execute_ms.count() > max) { max = execute_ms.count(); }

        std::cout  << "avg execute = " << all / i << " ms | min execute = " << min << " ms | max execute = " << max << " ms\n";

        if (execute_ms.count() < 1000) {        
        sleep(1);       
        } 

        system("clear");

        if (i >= 10800)
        {
            all = 0;
            i = 1;
            int min = 1000;
            int max = 0;
        }        
    }
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "ru_RU.UTF-8"); 


    if (argc == 1)
    {
        std::cout << "no arguments!\nUse command: help\n";
        return -1;
    }
    else if (argc > 2)
    {
        std::cout << "too mony arguments\nUse command: help\n";
        return -1;
    }

    Commands ch = getCommand(argv[1]);

    // пошли запросики
    switch (ch)
    {
        case(help): {
            system("clear");

            std::cout << "\n\t" << CONSTANTS::core_version << "\n";
            std::cout << "\t\t\tList of commands: \n\n";
            std::cout << " ivr \t\t\t - кто в IVR \n";
            std::cout << " queue \t\t\t - текущая очередь \n";
            std::cout << " active_sip \t\t - какие активные sip зарегистрированы в очереди \n";
            std::cout << " connect_bd \t\t - проверка соединения с БД\n";
            std::cout << " start \t\t\t - сбор данных в БД (самая главная команда для полноценной работы)\n";   
            std::cout << " statistics \t\t - отобразить статистику\n";
            std::cout << " housekeeping \t\t - внутренния задания на очистку БД таблиц (queue, logging, ivr)\n\n";
            
            std::cout << "\t\t\t\t\t\t\t\t === by Petrov Yuri @2024 === \n\n";
            break;
        }
        case(ivr): {                // запись в БД кто сейчас слушает IVR 
            // запрос
            getIVR();   
            break;
        }
        case(queue): {              // запись в БД кто ушел из IVR в очередь
            // запрос
            getQueue();
            break;
        } 
        case(active_sip): {         // запись в БД кто сейчас с кем разговариваети сколько по времени
            // запрос
            getActiveSip();
            break;
        }
        case(connect_bd): {      
            SQL_REQUEST::SQL base;
            if (base.isConnectedBD()) {
                std::cout << "Connect UP\n";
            }
            else {
                std::cout << "Connect DOWN!\n";                
            }             
            break;
        }        
        case(start):      {         
            collect();
            break;
        }    
        case(statistics): {
            stat();
            break;
        }
        case(remote): {
           //REMOTE_COMMANDS::Remote remote;            
            //remote.chekNewCommand() ? std::cout << "New command EXIST\n" : std::cout << "New command NO EXIST\n";         
            break;
        }
        case(housekeeping): {
            // DEBUG дописать!
            HOUSEKEEPING::HouseKeeping task;           
            // task.createTask(HOUSEKEEPING::TASKS::TaskQueue); OK!
            // task.createTask(HOUSEKEEPING::TASKS::TaskLogging); OK!
            task.createTask(HOUSEKEEPING::TASKS::TaskIvr);
            break;
        }
        case(test): {
            REMOTE_COMMANDS::Remote remote;
            if (remote.getCountCommand())
            {
                remote.startCommand();
            }            
        }
    }
     return 0;
};