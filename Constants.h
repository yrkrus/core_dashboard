//  CONSTANTS VALUE

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

#if defined(_MSC_VER)
    #define METHOD_NAME std::string(__FUNCSIG__)
#else
    #define METHOD_NAME (std::string(__PRETTY_FUNCTION__).c_str())
#endif

#define BUILD __DATE__ " " __TIME__



namespace CONSTANTS
{   
    namespace TIMEOUT
    {
        static const uint32_t IVR               = 1000;   // timeout для проверки новых данных для ivr
        static const uint32_t QUEUE             = 1000;   // timeout для проверки новых данных для queue
        static const uint32_t ACTIVE_SESSION    = 1000;   // timeout для проверки новых данных для активной сеcсии операторов
        static const uint32_t OPERATOR_STATUS   = 100;    // timeout для проверки новых данных для смены статуса оператором
        static const uint32_t CLEARING_CURRENT_DAY = 60000; // timeout для проверки нужно ли очищать текущий день таблицы history_*
        static const uint32_t CHECK_TRUNK_SIP = 5000;     // timeout для проверки активности trunk sip
        static const uint32_t CHECK_INTERNAL = 10000;     // timeout для проверки внутренних проверок
    }

    namespace SERVER 
    {
      //  static const uint16_t PORT = 12345;
        static const unsigned int PORT = 12346;        
    }

    namespace LOG 
    {
        static const std::string IVR                = "ivr.log";
        static const std::string QUEUE              = "queue.log";
        static const std::string ACTIVE_SESSION     = "active_session.log";        
        // to_history
        static const std::string HISTORY_IVR        = "history_ivr.log";
        static const std::string HISTORY_QUEUE      = "history_queue.log";
        static const std::string HISTORY_LOGGING    = "history_logging.log";
        static const std::string HISTORY_ONHOLD     = "history_onhold.log";
        static const std::string HISTORY_SMS        = "history_sms.log";

        static const std::string REMOTE_COMMANDS    = "remote_commands.log";  
        
        static const std::string CHECK_TRUNK_SIP    = "check_trunk.log";
        static const std::string HTTP_REQUEST       = "http_request.log";
        static const std::string CALL_INFO          = "call_info.log";
    }
    
    // версия ядра
    namespace VERSION
    {          
        static const std::string CORE = "CORE | version 3.1 " BUILD; 
    }

    
       
    // *v0@|48lg*se  
    
    
    //valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=med --log-file=./core.log ./core_dashboard start
    //valgrind --leak-check=full --leak-resolution=med --log-file=./core.log ./core_dashboard start
    //
    //mysqldump -uzabbix -pUFGq4kZMNIMwxTzV -h10.34.222.19 dashboard > /root/core_dashboards/backup_bd/dashboard_20250730.sql


    //asterisk -rx "core show channels verbose" | grep to-atsaero5005  кто сейчас разговаривает с бабой железной

    //// запуск gdbserver
    //scl enable gcc-toolset-10 bash
    //gdbserver :7777 ./core_dashboard start

    

    // при совершении звонка, в ivr добавить еще id_звонка из queue таблицы,
    // потом будет проще найти этот звонок + повторные звонки будут проще отслеживаться

    // бэкапы базы будут тут жить \\srvbak\G$\dashboard_backup_COV

// для будущей интеграции с телефонами!!!    
 //  
 //   // перезагрузка
 //   https://admin:asz741@10.34.42.47/servlet?key=Reboot
 //   // набор номера
 //   http://admin:asz741@10.34.42.47/servlet?key=number=xxx&outgoing_uri=y 	Набор номера xxx
 //   // завершить вызов
 //   http://admin:asz741@10.34.42.47/servlet?key=CALLEND 
 //   // скриншот
 //   https://admin:asz741@10.34.42.47/servlet?m=mod_action&command=screenshot
 //   // регистрация
 //   https://admin:asz741@10.34.42.47/servlet?phonecfg=set[&account.1.label=64197][&account.1.display_name=64197][&account.1.auth_name=64197][&account.1.user_name=64197][&account.1.password=1240]
 //	
	//Лейбл 			 = &account.1.label=XXX
	//Отображаемое имя = &account.1.display_name=XXX
	//Имя регистрации  = &account.1.auth_name=XXX
	//Имя пользователя = &account.1.user_name=XXX
	//Пароль 			 = &account.1.password=XXX
 //  

	///var/spool/asterisk/outgoung/*.call

	//*.call
	//Channel: SIP/druOUT_220220/79275052333
	//CallerID: "Dasboard Missed Call" <79275052333>
	//WaitTime: 20
	//Extension: 64197
	//Priority: 1      

 //   когда через активные сессия сброс, если оператор в очереди то убираем его из нее!!!

};


#endif //CONSTANTS_H
