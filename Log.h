//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//			  логирование действий   				//
//													//	
//////////////////////////////////////////////////////

#pragma once
#include <string>


namespace LOG {
	
	enum class Log
	{
        Log_enter                   = 0,         // ¬ход
        Log_exit                    = 1,         // ¬ыход
        Log_auth_error              = 2,         // не успешна€ авторизаци€
        Log_exit_force              = 3,         // ¬ыход (через команду force_closed)
        Log_add_queue_5000          = 4,         // добавление в очередь 5000
        Log_add_queue_5050          = 5,         // добавление в очередь 5050
        Log_add_queue_5000_5050     = 6,         // добавление в очередь 5000 и 5050
        Log_del_queue_5000          = 7,         // удаление из очереди 5000
        Log_del_queue_5050          = 8,         // удаление из очереди 5050
        Log_del_queue_5000_5050     = 9,         // удаление из очереди 5000 и 5050
        Log_available               = 10,        // доступен
        Log_home                    = 11,        // домой        
        Log_exodus                  = 12,        // исход
        Log_break                   = 13,        // перерыв
        Log_dinner                  = 14,        // обед
        Log_postvyzov               = 15,        // поствызов
        Log_studies                 = 16,        // учеба
        Log_IT                      = 17,        // »“
        Log_transfer                = 18,        // переносы
        Log_reserve                 = 19,        // резерв
	};
	
	class Logging
	{
    public:
        Logging()	= default;
		~Logging()	= default;
    
        void createLog(Log command, int base_id);       // создание лога
	};
}




