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
#include <unordered_map>


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

		//void query_test();
		//void insertData_test();

		//table IVR
	//	void insertIVR(const char *phone, const char *time, std::string callerid );			// ���������� ������ � ������� IVR
	//	bool isExistIVRPhone(const char *phone);											// ���������� �� ����� ��� ����� � ������� IVR
	//	int getLastIDphone(const char *phone);												// ��������� ���������� ID �����������
	//	void updateIVR(const char *id, const char *phone, const char *time);				// ���������� ������ � ������� IVR
		


		// table QUEUE
		//void insertQUEUE(const char *queue, const char *phone, const char *time);			// ���������� ������ � ������� QUEUE
		//bool isExistQUEUE(const char *queue, const char *phone);							// ���������� �� ����� ��� ����� � ������� QUEUE
		//void updateQUEUE(const char *id, const char *phone, const char *time);				// ���������� ������ � ������� QUEUE
		//int getLastIDQUEUE(const char *phone);												// ��������� ���������� ID �����������
		//void updateQUEUE_SIP(const char *phone, const char *sip, const char *talk_time);	// ���������� ������ ������� QUEUE � ��� � ��� ������ ������������� ��������
		//bool isExistQUEUE_SIP(const char *phone);											// ���������� �� ����� ����� � ������� QUEUE ����� �������� sip ��������� ������� � �������� �����
		//void updateQUEUE_fail(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// ���������� ������ ����� ������ �� �������� ����� ������� 
		//void updateQUEUE_fail();															// ���������� ������ ����� ������ �� �������� ����� ������� 
		//void updateIVR_to_queue(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// ���������� ������ ����� � ��� ������ �� IVR ����� � �������
		//bool isExistQueueAfter20hours();													// �������� ���� �� ������ ������� ��������� ����� 20:00
		//void updateQUEUE_hash(const std::vector<QUEUE_OLD::Pacients_old> &pacient_list);			// ���������� ���� hash ����� ������� ����������
		//bool isExistAnsweredAfter20hours();													// �������� ���� �� ������ ������� ����� ���������� ������ �������� ����� ���� ��� �������� ���� �� �����		
		//void updateAnswered_fail();															// ���������� ������ ����� �������� ��������� � ���� �� �����, � ������ ��� ��� ��������� �� ������������


		// table operators_queue
		//void insertOperatorsQueue(const char *sip, const char *queue);						// ���������� sip ������ ��������� + ��� �������
		//void clearOperatorsQueue();															// ������� ������� operators_queue
		//bool isExistOperatorsQueue_old(const char *sip, const char *queue);						// �������� ���������� �� ����� sip+������� � ��
		//bool isExistOperatorsQueue_old();														// �������� ���������� �� ���� 1 ������ � �� sip+�������
		//void checkOperatorsQueue(const std::vector<ACTIVE_SIP_old::Operators_old> &list_operators);	// �������� ������� sip + ������� 
		//void deleteOperatorsQueue(const std::string &sip);											// �������� sip ������ ��������� �� ���� ��������
		//void deleteOperatorsQueue(const std::string &sip, const std::string &queue);						// �������� sip ������ ��������� � ���������� �������


		// ����������
		int getIVR_totalCalls();															// ������� ����� ��������� �� ����� IVR
		//int getIVR_totalCalls(const IVR_OLD::CallerID &trunk);									// ������� ����� ��������� �� ����� IVR (����� �� trunk)
		int getQUEUE_Calls(bool answered);													// ������� ����� �������� � ������� �����������
		int getQUEUE_Calls();																// ������� ����� ���� � �������

		// RemoteComands
		bool remoteCheckNewCommads();														// �������� ���� ����� ������� ���  �����\������ �� �������
		void createListRemoteCommands(std::vector<REMOTE_COMMANDS_old::R_Commands_old> &list);	    // ��������� ������� ��������� ������
		void startRemoteCommand(int id, std::string sip, remote::ecCommand command, int user_id);	// ������ ��������� �������
		void deleteRemoteCommand(int id);													// �������� ������� ����������� �������
		void updateStatusOperators(int user_id, remote::ecStatusOperator status);	// ���������� �������� ������� ���������


		// ����������� 
		void addLog(remote::ecCommand command, int base_id);    // �������� ���� � ��
		
		
		// Housekeeping
		void execTaskQueue();																// ���������� ������ ������� ������� queue
		void execTaskLogging();																// ���������� ������ ������� ������� Logging
		void execTaskIvr();																	// ���������� ������ ������� ������� Ivr
		void execTaskOnHold();																// ���������� ������ ������� ������� operators_onhold
		void execTaskSmsSending();															// ���������� ������ ������� ������� sms_sending	
		bool insertDataTaskQueue(HOUSEKEEPING::Queue_old &queue);								// ������� ������ ��� ������� history_queue			
		bool deleteDataTaskQueue(int ID);													// �������� ������ �� ������� queue
		bool insertDataTaskLogging(HOUSEKEEPING::Logging &logging);							// ������� ������ ��� ������� history_logging
		bool deleteDataTaskLogging(int ID);													// �������� ������ �� ������� logging
		bool insertDataTaskIvr(HOUSEKEEPING::IVR_ &ivr);										// ������� ������ �� ������ history_ivr
		bool deleteDataTaskIvr(int ID);														// �������� ������ �� ������� ivr 
		bool insertDataTaskOnHold(HOUSEKEEPING::OnHold &onHold);							// ������� ������ ��� ������� history_onhold
		bool deleteDataTaskOnHold(int ID);													// �������� ������ �� ������� operatorsd_onhold
		bool insertDataTaskSmsSending(const HOUSEKEEPING::SmsSending &_smsSending);		    // ������� ������ ��� ������� history_sms_sending
		bool deleteDataTaskSmsSending(int _id);	  											// �������� ������ �� ������� sms_sending


		// table operators_ohhold
		//void updateOperatorsOnHold(ACTIVE_SIP_old::Parsing_old *list);						// �������� ������ � ������� ��������� OnHold
		//std::shared_ptr<std::vector<ACTIVE_SIP_old::OnHold_old>> createOnHoldSip();								    // �������� ������ � sip ����������� ������� ��������� � ������� OnHold �� ��
		//void addOperatorsOnHold(const std::string &sip, const std::string &phone);				// ���������� sip ������ ��������� � ������� operators_onhold
		//void disableOperatorsOnHold(const ACTIVE_SIP_old::OnHold_old &onhold);						// ���������� ������ ��� sip �������� ���� � ������� onHold 
		//bool isExistOnHold(const std::string &sip, std::string hash);							// �������� ���������� �� ����� ��� onHold (��� �� ������������� �� �����)											


	private:
		MYSQL mysql;
		void createMySQLConnect(MYSQL &mysql);												// ������������ � �� MySQL
	};	

};


#endif //SQLREQUEST_H