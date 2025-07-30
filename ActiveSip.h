#ifndef ACTIVESIP_H
#define ACTIVESIP_H

#include <string>
#include <vector>
#include "IAsteriskData.h"
#include "ISQLConnect.h"
#include "Queue.h"


static std::string SESSION_SIP_RESPONSE		= "asterisk -rx \"core show channels concise\"";
static std::string SESSION_QUEUE_RESPONSE	= "asterisk -rx \"queue show %queue\"";

class Queue;
using SP_Queue = std::shared_ptr<Queue>;
enum class EQueueNumber;
using QueueList = std::vector<EQueueNumber>;


namespace active_sip 
{
	// ��������� ��������� 
	struct Operator 
	{
		std::string sipNumber = "null";		// ����� sip ���������
		QueueList	queueList;				// ������� � ������� ����� ���������
		bool isOnHold = false ;             // ��������� �� �������� � ������� OnHold
		std::string phoneOnHold = "null";	// ������� � ������� ���� onHold		
	};
	using OperatorList = std::vector<Operator>;
	
	// ��������� �������� ������
	struct ActiveCall 
	{
		std::string phone = "null";			// ������� ����� �������� � ������� ������� ������
		std::string sip = "null";			// ���������� sip ������� ����� ������
		std::string talkTime = "null";		// ����� ����������  //TODO ����� � int ����������		
	};
	using ActiveCallList = std::vector<ActiveCall>;

	// ��������� onHold
	struct OnHold 
	{
		int id = 0;						// id �� ��
		std::string sip = "null";		// sip � ������� ��� ��������
		std::string phone = "null";		// ������� 		
	
		inline bool check() const noexcept
		{
			if (id == 0 ||
				sip.find("null") != std::string::npos ||
				phone.find("null") != std::string::npos)
			{				
				return false;
			}

			return true;
		}
	};
	using OnHoldList = std::vector<OnHold>;


	class ActiveSession : public IAsteriskData	// ����� � ������� ����� ���� ������ �� �������� ������� ���������� 
	{
	public:
		ActiveSession(SP_Queue &_queue);
		~ActiveSession() override;

		void Start() override;
		void Stop() override;
		void Parsing() override;				// ������ ����� ������
		
	private:
		SP_Queue	&m_queueSession;	// ������ �� �������
		
		OperatorList	m_listOperators;	// TODO ����� ����� � shared_ptr ����� ��������
		ActiveCallList	m_listCall;			// TODO ����� ����� � shared_ptr ����� ��������
		SP_SQL			m_sql;		
		IFile			m_queue;			// ������ ���������� �� ������� ��������
		Log				m_log;


		void CreateListActiveSessionOperators();			// �������� ��������� � �����
		void CreateListActiveSessionCalls();				// �������� ������ � �����

		void CreateActiveOperators(const EQueueNumber _queue);	// ������ �������� ���������� � �����
		void CreateOperator(const std::string &_lines, Operator &, EQueueNumber);	// �������� ��������� Operator					
		std::string FindSipNumber(const std::string &_lines);	// ������� ���������� ��������� sip ���������
		bool FindOnHoldStatus(const std::string &_lines);		// ������� ���������� ������� onHold
		
		void InsertAndUpdateQueueNumberOperators(); // ����������\���������� ������ ������� ��������� � ��
		bool IsExistListOperators();		// ���� �� ������ � m_listOperators
		bool IsExistListOperatorsOnHold();	// ���� �� ������ � m_listOperators.onHold
		bool IsExistListCalls();			// ���� �� ������ � m_listCall

		bool IsExistOperatorsQueue();	// ���������� �� ���� 1 ������ � �� sip+�������
		bool IsExistOperatorsQueue(const std::string &_sip, const std::string &_queue);	// ���������� �� ���� ������ � �� sip+�������
		void ClearOperatorsQueue();		// ������� ������� operators_queue
		void CheckOperatorsQueue();		// �������� ���� �� �������� ��� � �������
		bool GetActiveQueueOperators(OperatorList &_activeList, std::string &_errorDescription); // �������� ������� ���������� � ��
		void DeleteOperatorsQueue(const std::string &_sip, const std::string &_queue);	// �������� ������� ��������� �� �� ������� operators_queue
		void DeleteOperatorsQueue(const std::string &_sip);								// �������� ������� ��������� �� �� ������� operators_queue ���� sip
		void InsertOperatorsQueue(const std::string &_sip, const std::string &_queue);	// ���������� ������� ��������� � �� ������� operators_queue
	
		bool CreateActiveCall(const std::string &_lines, const std::string &_sipNumber, ActiveCall &_caller); // ������� � ���������� ��������� ������ � ������� ������������� ��������
		bool CheckActiveCall(const ActiveCall &_caller); // �������� ������������ ��������� ������

		void UpdateActiveSessionCalls(); // ���������� ������� ������� ����������
		
		void UpdateTalkCallOperator();								// ���������� ������ ������� queue � ��� � ��� ������ ������������� ��������
		bool IsExistTalkCallOperator(const std::string &_phone);	// ���������� �� ����� ����� � ������� queue ����� �������� sip ��������� ������� � �������� �����
		int  GetLastTalkCallOperatorID(const std::string &_phone);	// ��������� ���������� ID ����������� ��������� �������� ��������� � ������� queue
	
		// onHold 
		void UpdateOnHoldStatusOperator();					// ���������� ������� onHold
		void AddPhoneOnHoldInOperator(Operator &); // ���������� ������ �������� ������� �� onHold ������		
		bool GetActiveOnHold(OnHoldList &_onHoldList, std::string &_errorDescription);	// ��������� ���� onHold �������� ������� ���� � ��
		
		void DisableOnHold(const OnHoldList &_onHoldList);		// ������� ���� ���������� ������� � ������� onHold �� �� 
		bool DisableHold(const OnHold &_hold, std::string &_errorDescription);	// ���������� onHold � ��
		bool AddHold(const Operator&, std::string &_errorDescription);	// ���������� ������ onHold � ��

		void CheckOnHold(OnHoldList &_onHoldList);		// �������� �������� ����������\���������� onHold 
	};
}
using SP_ActiveSession = std::shared_ptr<active_sip::ActiveSession>;

#endif // ACTIVESIP_H
