//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//		���������� ������� �� �������� � ������ 	//
//													//	
//////////////////////////////////////////////////////

#include <string>
#include <list>
#include <unordered_map>
#include "Constants.h"
#include "SQLRequest.h"
#include "Queue.h"
#include "RemoteCommands.h"
#include "Log.h"
#include <mysql/mysql.h>


#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

namespace INTERNALFUNCTION {
	
	typedef std::vector<ACTIVE_SIP::OnHold> OnHold;
	typedef std::vector<ACTIVE_SIP::Operators> Operators;

	std::string phoneParsing(std::string &phone);					// ������� ������ �������� � ���������� ���
	void getIVR();												    // ������� + �������� ������� IVR
	void getQueue(void);											// ������� + �������� ������� �������
	void getActiveSip(void);										// ������� + �������� ��� � ��� �������������
	std::string getNumberQueue(CONSTANTS::AsteriskQueue queue);		// ��������� ������ �������
	std::string getTalkTime(std::string talk);						// ������� �������� �� ��� -> 00:00:00

	// ������� ������ �� �������� (������ ���-�����-���� 00:00:00 )
	std::string getCurrentDateTime();							// ������� ����� 
	std::string getCurrentStartDay();							// ������� ������ ���
	std::string getCurrentDateTimeAfterMinutes(int minutes);	// ������� ����� -(�����) ��������� ������ 
	std::string getCurrentDateTimeAfter20hours();				// ������� ����� ����� 20:00 

	void showVersionCore(unsigned int iter);					// ����� ������ ����

	// ���������� // ���� ��� ������, ����� ����� � ��������� ����� �������
	void getStatistics();

	void showErrorBD(const std::string str); // ������������ ���� ��� �� �������� ������������ � ��
	void showErrorBD(const std::string str, MYSQL *mysql); // ����������� ���� ��� ����� ����� �� ������

	LOG::Log getRemoteCommand(int command);	// �������������� ������� ��������� ������ �� int -> LOG::Log
	int getRemoteCommand(LOG::Log command); // �������������� ������� ��������� ������ �� LOG::Log -> int
	int getStatusOperators(REMOTE_COMMANDS::StatusOperators status); // �������������� ������� ��������� ������ �� REMOTE_COMMANDS::StatusOperators -> int
	bool isExistNewOnHoldOperators(const OnHold *onHold, const Operators &operators); // �������� ���� �� ������� ����� onHold �� �� � active_sip->onHold
	std::unordered_map <std::string, std::string> *createNewOnHoldOperators(const OnHold &onHold, const Operators &operators);			// �������� ������ � ������ ����������� � OnHold


	bool remoteCommandChekedExecution(LOG::Log command);	// �������� ������� �� ��������� ��������� �������

	bool to_bool(std::string str);						// ��������� �� std::string -> bool
	std::string to_string(bool value);					// ��������� �� bool -> std::string 

	size_t string_to_size_t(const std::string &str);	// ��������� std::string->size_t
}
	

#endif //INTERNALFUNCTION_H