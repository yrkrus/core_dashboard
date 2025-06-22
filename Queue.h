  
//	      ������� ������� ���������� � �������		

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef QUEUE_H
#define QUEUE_H

static std::string QUEUE_COMMANDS		= "Queue";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";


class Queue : public IAsteriskData
{
public:
	
	enum class ecQueueNumber 
	{
		eUnknown = 0,
		e5000,
		e5050,
		e5100,
	};

	struct QueueCalls
	{
		std::string phone	= "null";					// ������� ����� �������� ������� � ������� ������
		std::string waiting = "null";					// ����� � (���) ������� ������ � ������� ���������
		ecQueueNumber queue	= ecQueueNumber::eUnknown;	// ����� �������
	};

	struct CallsInBase	// ��������� �� ��
	{
		std::string id			= "- 1";
		std::string phone		= "null";
		std::string date_time	= "null";
		size_t hash				= 0;
	};
	typedef std::vector<CallsInBase> CallsInBaseList;
	
	Queue();
	~Queue() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// ������ ����� ������

private:
	std::vector<QueueCalls>	m_listQueue;
	SP_SQL					m_sql;
	
	
	bool FindQueueCallers();									// ����� ������� �������� �������

	bool CreateQueueCallers(const std::string&, QueueCalls&);
	bool CheckCallers(const QueueCalls&);							// �������� ������������ �������� ������

	bool IsExistQueueCalls();										// ���� �� ������ � ������

	ecQueueNumber StringToEnum(const std::string &_str);
	std::string EnumToString(ecQueueNumber _number);

	void InsertQueueCalls();													// ���������� ������ � ��
	void InsertCall(const QueueCalls &_call);									// ���������� ������ ������
	bool UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription); // ���������� ������������� ������
	void UpdateCallFail(const std::vector<QueueCalls> &_calls);					// ���������� ������ ���� ������ ��� � �������, �� �� �������� ������ �� ���������
	void UpdateCallFail();														// ���������� ������ ���� ������ ��� � �������, �� �� �������� ������ �� ���������
	void UpdateCallToIVR(const std::vector<QueueCalls> &_calls);				// ���������� ������ ����� � ��� ������ �� IVR ����� � �������
	void UpdateCallSuccess(const std::vector<QueueCalls> &_calls);				// ���������� ������ ����� �������� ������� ���������

	bool IsExistCall(ecQueueNumber _queue, const std::string &_phone);			// ���� �� ��� ����� ����� � ��
	int GetLastQueueCallId(const std::string &_phone);							// id ������ �� �� � ������

	bool GetCallsInBase(CallsInBaseList &_vcalls, const std::vector<QueueCalls> &_queueCalls, std::string &_errorDescription); // ��������� ������� �� �� 
	bool GetCallsInBase(CallsInBaseList &_vcalls, std::string &_errorDescription);						// ��������� ������� �� �� 

	bool IsExistCallAfter20Hours(std::string &_errorDescription);			// ���� �� ������ ����� 20:00
	void UpdateCallsAfter20hours();											// ���� ��������� ������� ������� ����� 20:00, �������� ��

	bool IsExistCallAnsweredAfter20hours(std::string &_errorDescription);	// ���� �� �� ��� hash'���� ������, ����� �������� ��� �������� �������� � ���� �� �����
	void UpdateCallAnsweredAfter20hours();									// ���� �� ��� hash'���� ������ ��������� ��

	


};



namespace QUEUE_OLD
{
	enum Currentfind
	{
		phone_find,
		waiting_find,
		queue_find,
	};

	struct Pacients_old
	{
		std::string phone	{ "null" };		// ������� ����� �������� ������� � ������� ������
		std::string waiting { "null" };		// ����� � (���) ������� ������ � ������� ���������
		std::string queue	{ "null" };		// ����� �������
	};

	class Parsing
	{
	public:
		Parsing(const char *fileQueue);
		~Parsing() = default;

		bool isExistList();					// ���� �� �������	
		void show(bool silent = false);
		
		void insertData();					// ���������� ������ � ��
		bool isExistQueueAfter20hours();	// �������� ���� �� �� ���������� ������ ����� 20:00
		void updateQueueAfter20hours();		// ���������� ������ ���� ������ ������ ���� ��� ��� �������� ���������� �� �����	
		bool isExistAnsweredAfter20hours(); // �������� ���� �� ������ �� ������� ��������� �������������, �� �� ������ ����������
		void updateAnsweredAfter20hours();	// ���������� ������ ����� ��������� ������������� � ���� � �����
		

	private:
		std::string findParsing(std::string str, QUEUE_OLD::Currentfind find); // �������  
		std::vector<QUEUE_OLD::Pacients_old> pacient_list;
	};


	struct BD 
	{
		std::string id;
		std::string phone;
		std::string date_time;
		size_t hash{0};
	};

	class QueueBD_old 
	{
		public:
			QueueBD_old()	= default;
			~QueueBD_old()	= default;
			std::vector<QUEUE_OLD::BD> list;
	};
}

#endif //QUEUE_H
