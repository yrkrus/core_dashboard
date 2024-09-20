//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 04.06.2024				//
//				   SQL �������	             		//
//													//	
//////////////////////////////////////////////////////
#include <iostream>
#include "Constants.h"
#include <mysql/mysql.h>
#include "IVR.h"
#include "Queue.h"
#include "ActiveSip.h"
#include "RemoteCommands.h"
#include "Log.h"
#include "HouseKeeping.h"
#include <vector>
#include <memory>


#ifndef SQLREQUEST_H
#define	SQLREQUEST_H

namespace SQL_REQUEST
{
	class SQL
	{
	public:
		SQL();
		~SQL() = default;

		bool isConnectedBD();	// ���� �� ������� � ��

		void query_test();
		void insertData_test();

		//table IVR
		void insertIVR(const char *phone, const char *time, std::string callerid );			// ���������� ������ � ������� IVR
		bool isExistIVRPhone(const char *phone);											// ���������� �� ����� ��� ����� � ������� IVR
		int getLastIDphone(const char *phone);												// ��������� ���������� ID �����������
		void updateIVR(const char *id, const char *phone, const char *time);				// ���������� ������ � ������� IVR
		


		// table QUEUE
		void insertQUEUE(const char *queue, const char *phone, const char *time);			// ���������� ������ � ������� QUEUE
		bool isExistQUEUE(const char *queue, const char *phone);							// ���������� �� ����� ��� ����� � ������� QUEUE
		void updateQUEUE(const char *id, const char *phone, const char *time);				// ���������� ������ � ������� QUEUE
		int getLastIDQUEUE(const char *phone);												// ��������� ���������� ID �����������
		void updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time);	// ���������� ������ ������� QUEUE � ��� � ��� ������ ������������� ��������
		bool isExistQUEUE_SIP(const char *phone);											// ���������� �� ����� ����� � ������� QUEUE ����� �������� sip ��������� ������� � �������� �����
		void updateQUEUE_fail(const std::vector<QUEUE::Pacients> &pacient_list);			// ���������� ������ ����� ������ �� �������� ����� ������� 
		void updateQUEUE_fail();															// ���������� ������ ����� ������ �� �������� ����� ������� 
		void updateIVR_to_queue(const std::vector<QUEUE::Pacients> &pacient_list);			// ���������� ������ ����� � ��� ������ �� IVR ����� � �������
		bool isExistQueueAfter20hours();													// �������� ���� �� ������ ������� ��������� ����� 20:00
		void updateQUEUE_hash(const std::vector<QUEUE::Pacients> &pacient_list);			// ���������� ���� hash ����� ������� ����������
		bool isExistAnsweredAfter20hours();													// �������� ���� �� ������ ������� ����� ���������� ������ �������� ����� ���� ��� �������� ���� �� �����		
		void updateAnswered_fail();															// ���������� ������ ����� �������� ��������� � ���� �� �����, � ������ ��� ��� ��������� �� ������������


		// table operators_queue
		void insertOperatorsQueue(const char *sip, const char *queue);						// ���������� sip ������ ��������� + ��� �������
		void clearOperatorsQueue();															// ������� ������� operators_queue
		bool isExistOperatorsQueue(const char *sip, const char *queue);						// �������� ���������� �� ����� sip+������� � ��
		bool isExistOperatorsQueue();														// �������� ���������� �� ���� 1 ������ � �� sip+�������
		void checkOperatorsQueue(const std::vector<ACTIVE_SIP::Operators> &list_operators);	// �������� ������� sip + ������� 
		void deleteOperatorsQueue(std::string sip);											// �������� sip ������ ��������� �� ���� ��������
		void deleteOperatorsQueue(std::string sip, std::string queue);						// �������� sip ������ ��������� � ���������� �������


		// ����������
		int getIVR_totalCalls();															// ������� ����� ��������� �� ����� IVR
		int getIVR_totalCalls(const IVR::CallerID &trunk);									// ������� ����� ��������� �� ����� IVR (����� �� trunk)
		int getQUEUE_Calls(bool answered);													// ������� ����� �������� � ������� �����������
		int getQUEUE_Calls();																// ������� ����� ���� � �������

		// RemoteComands
		bool remoteCheckNewCommads();														// �������� ���� ����� ������� ���  �����\������ �� �������
		void createListRemoteCommands(std::vector<REMOTE_COMMANDS::R_Commands> &list);	    // ��������� ������� ��������� ������
		void startRemoteCommand(int id, std::string sip, LOG::Log command, int user_id);	// ������ ��������� �������
		void deleteRemoteCommand(int id);													// �������� ������� ����������� �������
		void updateStatusOperators(int user_id, REMOTE_COMMANDS::StatusOperators status);	// ���������� �������� ������� ���������


		// ����������� 
		void addLog(LOG::Log command, int base_id);    // �������� ���� � ��
		
		
		// Housekeeping
		void execTaskQueue();																// ���������� ������ ������� ������� queue
		void execTaskLogging();																// ���������� ������ ������� ������� Logging
		void execTaskIvr();																	// ���������� ������ ������� ������� Ivr
		bool insertDataTaskQueue(HOUSEKEEPING::Queue &queue);								// ������� ������ ��� ������� history_queue			
		bool deleteDataTaskQueue(int ID);													// �������� ������ �� ������� queue
		bool insertDataTaskLogging(HOUSEKEEPING::Logging &logging);							// ������� ������ ��� ������� history_logging
		bool deleteDataTaskLogging(int ID);													// �������� ������ �� ������� logging

	private:
		MYSQL mysql;
		void createMySQLConnect(MYSQL &mysql);												// ������������ � �� MySQL
	};	

};


#endif //SQLREQUEST_H