//  CONSTANTS VALUE

#include <string>

#ifndef CONSTANTS_H
#define CONSTANTS_H

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
        static const unsigned int ACTIVE_SESSION    = 1000;   // timeout ��� �������� ����� ������ ��� �������� ����� ����������
        static const unsigned int OPERATOR_STATUS   = 100;    // timeout ��� �������� ����� ������ ��� ����� ������� ����������
    }

    namespace SERVER 
    {
        static const unsigned int PORT = 12345;
        //static const unsigned int PORT = 12346;        
    }
    
    
    
    
    // DEBUG MODE 
    static bool DEBUG_MODE = false;      // ��� ���� ������ �� ��������� ������� �� ���������
    
    static bool SAFE_LOG        { true };       // ���������� �� � ��� ������ ����
    static bool LOG_MODE_INFO   { true };       // ����� ������ ���� INFO (��� ���� ������ ������������ ���������� ) 
    static bool LOG_MODE_DEBUG  { false };       // ����� ������ ���� DEBUG (��� ���� ������ ������������ ������� �� � �� ������) 
    static bool LOG_MODE_ERROR  { true };       // ����� ������ ���� ERROR (������� ���������, ����������� ��� ������)

    // WKroot#791  old
    // *v0@|48lg*se  new
   
    /*
    
    valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=med --log-file=./core.log ./core_dashboard start
    valgrind --leak-check=full --leak-resolution=med --log-file=./core.log ./core_dashboard start
    
    mysqldump -uzabbix -pUFGq4kZMNIMwxTzV -h10.34.222.19 dashboard > /root/core_dashboards/backup_bd/dashboard_20250716.sql


    asterisk -rx "core show channels verbose" | grep to-atsaero5005  ��� ������ ������������� � ����� ��������

    // ������ gdbserver
    scl enable gcc-toolset-10 bash
    gdbserver :7777 ./core_dashboard start

    */

    // ��� ���������� ������, � ivr �������� ��� id_������ �� queue �������,
    // ����� ����� ����� ����� ���� ������ + ��������� ������ ����� ����� �������������

    // ������ ���� ����� ��� ���� \\srvbak\G$\dashboard_backup_COV

// ��� ������� ���������� � ����������!!!    
/*   
    // ������������
    https://admin:asz741@10.34.42.47/servlet?key=Reboot
    // ����� ������
    http://admin:asz741@10.34.42.47/servlet?key=number=xxx&outgoing_uri=y 	����� ������ xxx
    // ��������� �����
    http://admin:asz741@10.34.42.47/servlet?key=CALLEND 
    // ��������
    https://admin:asz741@10.34.42.47/servlet?m=mod_action&command=screenshot
    // �����������
    https://admin:asz741@10.34.42.47/servlet?phonecfg=set[&account.1.label=64197][&account.1.display_name=64197][&account.1.auth_name=64197][&account.1.user_name=64197][&account.1.password=1240]
 	
	����� 			 = &account.1.label=XXX
	������������ ��� = &account.1.display_name=XXX
	��� �����������  = &account.1.auth_name=XXX
	��� ������������ = &account.1.user_name=XXX
	������ 			 = &account.1.password=XXX
   

	/var/spool/asterisk/outgoung/*.call

	*.call
	Channel: SIP/druOUT_220220/79275052333
	CallerID: "Dasboard Missed Call" <79275052333>
	WaitTime: 20
	Extension: 64197
	Priority: 1 
    */ 


    /*
    �����! �� ������
    ������ �� ����� ��������� �� ./root/core_dashboard/core_dashboard/core_dashboard

    ����� ����� �������� ������ �����, ���� �������� � ������� �� ������� ��� �� ���!!!

    */

 // ������ ����
/**
* 
* 
*   @param
*/
 static std::string core_version = "CORE DASHBOARD | version 2.17 " BUILD;
 

// asterisk �������  ������� �����
//enum AsteriskQueue
//{
//    main,       // �������� �������     5000
//    lukoil,     // ������� �����        5050
//    COUNT = 2,
//};


// ���������
// IVREXT - ���� �� ������� ������������
// ����� IVREXT -> Spasibo - ���� �� ������� ��� �������

// for IVR 
//static std::string cIVRCommands_old	    = "Playback|lukoil|ivr-3";
//static std::string cIVRCommandsEx1_old  = "IVREXT";
//static std::string cIVRCommandsEx2_old  = "Spasibo";
//static std::string cIVRName_old         = "IVR.txt";
//static std::string cIVRResponse_old     = "asterisk -rx \"core show channels verbose\" | grep -E \"" + cIVRCommands_old + "\" " + " | grep -v \"" + cIVRCommandsEx1_old + "\" " + " | grep -v \"" + cIVRCommandsEx2_old + "\" > " + cIVRName_old;

// for QUEUE
//static std::string cQueueCommands	= "Queue";
//static std::string cQueueCommandsEx	= "App";
//static std::string cQueueName		= "Queue.txt";
//static std::string cQueueResponse	= "asterisk -rx \"core show channels verbose\" | grep -E \"" + cQueueCommands + "\" " + " | grep -v \"" + cQueueCommandsEx + "\" > " + cQueueName;

// for ActiveSIP
//static std::string cActiveSipName				= "ActiveSip.txt";
//static std::string cActiveSipResponse			= "asterisk -rx \"core show channels concise\" > " + cActiveSipName;
//static std::string cActiveSipOperatorsName		= "ActiveSipOperators.txt";
//static std::string cActiveSipOperatorsResponse	= "asterisk -rx \"queue show %queue\" > " + cActiveSipOperatorsName;

//for MySQL Connect
static std::string cHOST       = "10.34.222.19";
static std::string cBD         = "dashboard";
static std::string cLOGIN      = "zabbix";
static std::string cPASSWORD   = "UFGq4kZMNIMwxTzV";

//for Remote Connect
//static std::string cRemoteCommandName           = "RemoteCommand.txt";
//static std::string cRemoteCommandResponseAdd    = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" > " + cRemoteCommandName;
//static std::string cRemoteCommandResponseDel    = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" > " + cRemoteCommandName;

// for FileLog (���� �����)
static std::string cFileLogDEBUG    = "Log_DEBUG.log";
static std::string cFileLogINFO     = "Log_INFO.log";
static std::string cFileLogERROR    = "Log_ERROR.log";

};


#endif //CONSTANTS_H
