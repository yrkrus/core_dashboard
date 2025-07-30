  
//	      ������� ������� ���������� � �������		

#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "Log.h"
#include "IAsteriskData.h"
#include "ISQLConnect.h"


static std::string QUEUE_COMMANDS		= "Queue|to-atsaero5005";
static std::string QUEUE_COMMANDS_EXT1	= "App";
static std::string QUEUE_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + QUEUE_COMMANDS + "\" " + " | grep -v \"" + QUEUE_COMMANDS_EXT1 + "\"";

enum class EQueueNumber // ����� � ������ bool Status::CreateCommand ������������ for 1..2 
{
	Unknown = 0,
	e5000,	
	e5050,
	e5000_e5050,	// ��������� 5000+5050 
	e5005,			// ������� ��� ���� ��������
};
using QueueList = std::vector<EQueueNumber>;

class Queue;
using SP_Queue = std::shared_ptr<Queue>;


class Queue : public IAsteriskData
{
public:	
	struct QueueCalls
	{
		std::string phone	= "null";					// ������� ����� �������� ������� � ������� ������
		std::string waiting = "null";					// ����� � (���) ������� ������ � ������� ���������
		EQueueNumber queue	= EQueueNumber::Unknown;	// ����� �������
	
		inline bool check() const noexcept
		{
			// ���� � phone ��� waiting ���� ��������� "null" 
			// ��� callerID == Unknown � ����� false
			if (phone.find("null")		!= std::string::npos ||
				waiting.find("null")	!= std::string::npos ||
				queue					== EQueueNumber::Unknown)
			{				 
				return false;
			}

			return true;
		}
	};
	using QueueCallsList = std::vector<QueueCalls>;
	
	struct CallsInBase	// ��������� �� ��
	{
		std::string id			= "-1";
		std::string phone		= "null";
		std::string date_time	= "null";
		size_t hash				= 0;
	};
	using CallsInBaseList = std::vector<CallsInBase>;

	
	Queue();
	~Queue() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// ������ ����� ������

	void UpdateCallSuccess();							// ���������� ������ ����� ��� �������� ���������� �� �����
	private:
	QueueCallsList		m_listQueue;
	SP_SQL				m_sql;
	Log					m_log;
	
	void UpdateCalls(const QueueCallsList &_callList);  			// ���������� �������

	bool FindQueueCallers();									// ����� ������� �������� �������

	bool CreateQueueCallers(const std::string&, QueueCalls&);
	bool CheckCallers(const QueueCalls&);						// �������� ������������ �������� ������

	bool IsExistQueueCalls();									// ���� �� ������ � ������

	void InsertQueueCalls();							// ���������� ������ � ��
	void InsertCall(const QueueCalls &_call);			// ���������� ������ ������
	void InsertCallVirtualOperator(const QueueCalls &_call);	// ���������� ������ ������ (����������� �������� ����)
	bool UpdateCall(int _id, const QueueCalls &_call, std::string &_errorDescription); // ���������� ������������� ������
	bool UpdateCallVirualOperator(int _id, const QueueCalls &_call, std::string &_errorDescription); // ���������� ������������� ������ (����������� ��������)
	void UpdateCallFail(const QueueCallsList &_calls);	// ���������� ������ ���� ������ ��� � �������, �� �� �������� ������ �� ���������
	//void UpdateCallFail();								// ���������� ������ ���� ������ ��� � �������, �� �� �������� ������ �� ���������
	void UpdateCallIvr(const QueueCallsList &_calls);	// ���������� ������ ����� � ��� ������ �� IVR ����� � ������� ��� �� ������������ ���������
	void UpdateCallIvrToQueue(const QueueCallsList &_calls);	// ������ �� IVR ����� � �������
	void UpdateCallIvrToVirtualOperator (const QueueCallsList &_calls);	// ������ �� IVR ����� �� ������������ ���������
	
	void UpdateCallSuccess(const QueueCallsList &_calls);				// ���������� ������ ����� �������� ������� ���������
	void UpdateCallSuccessRealOperator(const QueueCallsList &_calls);	// �������� ������� ��������� �������� ��������
	void UpdateCallSuccessVirtualOperator(const QueueCallsList &_calls);// �������� ������� ��������� ����������� ��������
	
	bool IsExistCall(EQueueNumber _queue, const std::string &_phone);	// ���� �� ��� ����� ����� � ��
	bool IsExistCallVirtualOperator(EQueueNumber _queue, const std::string &_phone);	// ���� �� ��� ����� ����� � �� (����������� ��������)

	int GetLastQueueCallId(const std::string &_phone);					// id ������ �� �� � ������
	int GetLastQueueVirtualOperatorCallId(const std::string &_phone);	// id ������ �� �� � ������(����������� ��������)

	bool GetCallsInBase(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription); // ��������� ������� �� �� 
	bool GetCallsInBase(CallsInBaseList &_vcalls, std::string &_errorDescription);						// ��������� ������� �� �� 
	bool GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, const QueueCallsList &_queueCalls, std::string &_errorDescription); // ��������� ������� �� �� (����������� ��������)
	bool GetCallsInBaseVirtualOperator(CallsInBaseList &_vcalls, std::string &_errorDescription);	// ��������� ������� �� �� (����������� ��������)

	//bool IsExistCallAfter20Hours(std::string &_errorDescription);			// ���� �� ������ ����� 20:00
	//void UpdateCallsAfter20hours();											// ���� ��������� ������� ������� ����� 20:00, �������� ��
	bool IsExistAnyAnsweredCall();					// ���� �� �� ��� hash'���� ������, ����� �������� ��� �������� �������� � ���� �� �����
	void UpdateAllAnyAnsweredCalls();				// ���� �� ��� hash'���� ������ ��������� �� (���!)

};


#endif //QUEUE_H
