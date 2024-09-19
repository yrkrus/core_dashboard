//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//		���������� ������� �� �������� � ������ 	//
//													//	
//////////////////////////////////////////////////////

#include <string>
#include "Constants.h"
#include "SQLRequest.h"
#include "Queue.h"
#include "RemoteCommands.h"
#include "Log.h"
#include <mysql/mysql.h>


#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

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

void showVersionCore();										// ����� ������ ����

// ���������� // ���� ��� ������, ����� ����� � ��������� ����� �������
void getStatistics();

void showErrorBD(const std::string str); // ������������ ���� ��� �� �������� ������������ � ��
void showErrorBD(const std::string str, MYSQL *mysql); // ����������� ���� ��� ����� ����� �� ������

LOG::Log getRemoteCommand(int command);	// �������������� ������� ��������� ������ �� int -> LOG::Log
int getRemoteCommand(LOG::Log command); // �������������� ������� ��������� ������ �� LOG::Log -> int
int getStatusOperators(REMOTE_COMMANDS::StatusOperators status); // �������������� ������� ��������� ������ �� REMOTE_COMMANDS::StatusOperators -> int



bool remoteCommandChekedExecution(LOG::Log command);	// �������� ������� �� ��������� ��������� �������



#endif //INTERNALFUNCTION_H