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
        static const unsigned int IVR               = 1000;   // timeout ��� �������� ����� ������ ��� ivr
        static const unsigned int QUEUE             = 1000;   // timeout ��� �������� ����� ������ ��� queue
        static const unsigned int ACTIVE_SESSION    = 1000;   // timeout ��� �������� ����� ������ ��� �������� ��c��� ����������
        static const unsigned int OPERATOR_STATUS   = 100;    // timeout ��� �������� ����� ������ ��� ����� ������� ����������
        static const unsigned int CLEARING_CURRENT_DAY = 60000; // timeout ��� �������� ����� �� ������� ������� ���� ������� history_*
    }

    namespace SERVER 
    {
       // static const unsigned int PORT = 12345;
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
    }
    
    
    // WKroot#791  old
    // *v0@|48lg*se  new  
    
    
    //valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=med --log-file=./core.log ./core_dashboard start
    //valgrind --leak-check=full --leak-resolution=med --log-file=./core.log ./core_dashboard start
    //
    //mysqldump -uzabbix -pUFGq4kZMNIMwxTzV -h10.34.222.19 dashboard > /root/core_dashboards/backup_bd/dashboard_20250726.sql


    //asterisk -rx "core show channels verbose" | grep to-atsaero5005  ��� ������ ������������� � ����� ��������

    //// ������ gdbserver
    //scl enable gcc-toolset-10 bash
    //gdbserver :7777 ./core_dashboard start

    

    // ��� ���������� ������, � ivr �������� ��� id_������ �� queue �������,
    // ����� ����� ����� ����� ���� ������ + ��������� ������ ����� ����� �������������

    // ������ ���� ����� ��� ���� \\srvbak\G$\dashboard_backup_COV

// ��� ������� ���������� � ����������!!!    
 //  
 //   // ������������
 //   https://admin:asz741@10.34.42.47/servlet?key=Reboot
 //   // ����� ������
 //   http://admin:asz741@10.34.42.47/servlet?key=number=xxx&outgoing_uri=y 	����� ������ xxx
 //   // ��������� �����
 //   http://admin:asz741@10.34.42.47/servlet?key=CALLEND 
 //   // ��������
 //   https://admin:asz741@10.34.42.47/servlet?m=mod_action&command=screenshot
 //   // �����������
 //   https://admin:asz741@10.34.42.47/servlet?phonecfg=set[&account.1.label=64197][&account.1.display_name=64197][&account.1.auth_name=64197][&account.1.user_name=64197][&account.1.password=1240]
 //	
	//����� 			 = &account.1.label=XXX
	//������������ ��� = &account.1.display_name=XXX
	//��� �����������  = &account.1.auth_name=XXX
	//��� ������������ = &account.1.user_name=XXX
	//������ 			 = &account.1.password=XXX
 //  

	///var/spool/asterisk/outgoung/*.call

	//*.call
	//Channel: SIP/druOUT_220220/79275052333
	//CallerID: "Dasboard Missed Call" <79275052333>
	//WaitTime: 20
	//Extension: 64197
	//Priority: 1 
 //    


 //   
 //   �����! �� ������
 //   ������ �� ����� ��������� �� ./root/core_dashboard/core_dashboard/core_dashboard

 //   ����� ����� �������� ������ �����, ���� �������� � ������� �� ������� ��� �� ���!!!

 //  

 // ������ ����
 //static std::string core_version = "CORE DASHBOARD | version 2.17 " BUILD;
 
};


#endif //CONSTANTS_H
