//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 19.09.2024				//
//			    ����������� ��������				//
//													//	
//////////////////////////////////////////////////////

#include <string>

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace CONSTANTS 
{
    // DEBUG MODE 
    static bool DEBUG_MODE{ false };
    // WKroot#791  old
    // *v0@|48lg*se  new
   
    /*
    
    valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=med --log-file=./core.log ./core_dashboard.out start
    valgrind --leak-check=full --leak-resolution=med --log-file=./core.log ./core_dashboard.out start
    
    mysqldump -uzabbix -pUFGq4kZMNIMwxTzV -h10.34.222.19 dashboard > /root/core_dashboards/backup_bd/dashboard_20240924.sql


    */


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
	CallerID: "Outgoing caller" <79275052333>
	WaitTime: 20
	Extension: 64197
	Priority: 1 
    */ 


    /*
    �����! �� ������
    ������ �� ����� ��������� �� ./root/core_dashboard/core_dashboard/core_dashboard.out
    */

 // ������ ����
 static std::string core_version = {"CORE DASHBOARD | version 2.13 bild 174265e "};
 

// asterisk �������  
enum AsteriskQueue
{
    main,       // �������� �������     5000
    lukoil,     // ������� �����        5050
    COUNT = 2,
};


// ���������
// IVREXT - ���� �� ������� ������������
// ����� IVREXT -> Spasibo - ���� �� ������� ��� �������

// for IVR
const std::string cIVRCommands	    = "Playback|lukoil|ivr-3";
const std::string cIVRCommandsEx1   = "IVREXT";
const std::string cIVRCommandsEx2   = "Spasibo";
const std::string cIVRName		    = "IVR.txt";
const std::string cIVRResponse	    = "asterisk -rx \"core show channels verbose\" | grep -E \"" + cIVRCommands + "\" " + " | grep -v \"" + cIVRCommandsEx1 + "\" " + " | grep -v \"" + cIVRCommandsEx2 + "\" > " + cIVRName;


// for QUEUE
const std::string cQueueCommands	= "Queue";
const std::string cQueueCommandsEx	= "App";
const std::string cQueueName		= "Queue.txt";
const std::string cQueueResponse	= "asterisk -rx \"core show channels verbose\" | grep -E \"" + cQueueCommands + "\" " + " | grep -v \"" + cQueueCommandsEx + "\" > " + cQueueName;

// for ActiveSIP
const std::string cActiveSipName				= "ActiveSip.txt";
const std::string cActiveSipResponse			= "asterisk -rx \"core show channels concise\" > " + cActiveSipName;
const std::string cActiveSipOperatorsName		= "ActiveSipOperators.txt";
const std::string cActiveSipOperatorsResponse	= "asterisk -rx \"queue show %queue\" > " + cActiveSipOperatorsName;

//for MySQL Connect
static std::string cHOST       = "10.34.222.19";
static std::string cBD         = "dashboard";
static std::string cLOGIN      = "zabbix";
static std::string cPASSWORD   = "UFGq4kZMNIMwxTzV";

//for Remote Connect
static std::string cRemoteCommandName           = "RemoteCommand.txt";
static std::string cRemoteCommandResponseAdd    = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" > " + cRemoteCommandName;
static std::string cRemoteCommandResponseDel    = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" > " + cRemoteCommandName;

};


#endif //CONSTANTS_H
