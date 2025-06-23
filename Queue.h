  
//	      ������� ������� ���������� � �������		

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>

#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef QUEUE_H
#define QUEUE_H

static std::string QUEUE_COMMANDS		= "Queue";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";

enum class ecQueueNumber
{
	eUnknown = 0,
	e5000,
	e5050,
	e5100,
};

class Queue : public IAsteriskData
{
public:	
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
	bool CheckCallers(const QueueCalls&);						// �������� ������������ �������� ������

	bool IsExistQueueCalls();									// ���� �� ������ � ������

	

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

template<typename T>
T StringToEnum(const std::string&);

template<>
inline ecQueueNumber StringToEnum<ecQueueNumber>(const std::string &_str)
{
	if (_str.find("5000") != std::string::npos)		return ecQueueNumber::e5000;
	if (_str.find("5050") != std::string::npos)		return ecQueueNumber::e5050;
	if (_str.find("5100") != std::string::npos)		return ecQueueNumber::e5100;

	return ecQueueNumber::eUnknown;
}

template<typename T>
std::string EnumToString(T);

template<>
inline std::string EnumToString<ecQueueNumber>(ecQueueNumber _number)
{
	static std::map<ecQueueNumber, std::string> queueNumber =
	{
		{ecQueueNumber::eUnknown,	"Unknown"},
		{ecQueueNumber::e5000,		"5000"},
		{ecQueueNumber::e5050,		"5050"},
		{ecQueueNumber::e5100,		"5100"},
	};

	auto it = queueNumber.find(_number);
	if (it != queueNumber.end())
	{
		return it->second;
	}
	return "Unknown";
}



//namespace QUEUE_OLD
//{
//	enum Currentfind
//	{
//		phone_find,
//		waiting_find,
//		queue_find,
//	};
//
//	struct Pacients_old
//	{
//		std::string phone	{ "null" };		// ������� ����� �������� ������� � ������� ������
//		std::string waiting { "null" };		// ����� � (���) ������� ������ � ������� ���������
//		std::string queue	{ "null" };		// ����� �������
//	};
//
//	class Parsing
//	{
//	public:
//		Parsing(const char *fileQueue);
//		~Parsing() = default;
//
//		bool isExistList();					// ���� �� �������	
//		void show(bool silent = false);
//		
//		void insertData();					// ���������� ������ � ��
//		bool isExistQueueAfter20hours();	// �������� ���� �� �� ���������� ������ ����� 20:00
//		void updateQueueAfter20hours();		// ���������� ������ ���� ������ ������ ���� ��� ��� �������� ���������� �� �����	
//		bool isExistAnsweredAfter20hours(); // �������� ���� �� ������ �� ������� ��������� �������������, �� �� ������ ����������
//		void updateAnsweredAfter20hours();	// ���������� ������ ����� ��������� ������������� � ���� � �����
//		
//
//	private:
//		std::string findParsing(std::string str, QUEUE_OLD::Currentfind find); // �������  
//		std::vector<QUEUE_OLD::Pacients_old> pacient_list;
//	};
//
//
//	struct BD 
//	{
//		std::string id;
//		std::string phone;
//		std::string date_time;
//		size_t hash{0};
//	};
//
//	class QueueBD_old 
//	{
//		public:
//			QueueBD_old()	= default;
//			~QueueBD_old()	= default;
//			std::vector<QUEUE_OLD::BD> list;
//	};
//}

#endif //QUEUE_H
