//  ������� �����������\�������������� � �������	

#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

#include "Log.h"
#include <vector>
#include <string>

static std::string COMMAND_ADD_QUEUE = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" ";
static std::string COMMAND_DEL_QUEUE = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" ";


namespace remote
{
	// ��������� ������� (ID ������ ����� �� ��� � � ��)
	enum class ECommand
	{
		Enter				= 0,         // ����
		Exit				= 1,         // �����
		AuthError			= 2,         // �� �������� �����������
		ExitForce			= 3,         // ����� (����� ������� force_closed)
		AddQueue5000		= 4,         // ���������� � ������� 5000
		AddQueue5050		= 5,         // ���������� � ������� 5050
		AddQueue5000_5050	= 6,         // ���������� � ������� 5000 � 5050
		DelQueue5000		= 7,         // �������� �� ������� 5000
		DelQueue5050		= 8,         // �������� �� ������� 5050
		DelQueue5000_5050	= 9,         // �������� �� ������� 5000 � 5050
		Available			= 10,        // ��������
		Home				= 11,        // �����        
		Exodus				= 12,        // �����
		Break				= 13,        // �������
		Dinner				= 14,        // ����
		Postvyzov			= 15,        // ���������
		Studies				= 16,        // �����
		IT					= 17,        // ��
		Transfer			= 18,        // ��������
		Reserve				= 19,        // ������
		Callback			= 20,        // callback
	};
		
	enum class ECommandType
	{
		Unknown = -1,
		Del,		// ������� �� ���������� � �������
		Add			// ������� �� �������� �� �������
	};
	
	enum class EStatus
	{
		Unknown		= -1,	// �����������
		Available	= 1,	// ��������
		Home		= 2,	// �����		
		Exodus		= 3,	// �����
		Break		= 4,	// �������
		Dinner		= 5,	// ����
		Postvyzov	= 6,	// ���������
		Studies		= 7,	// �����
		It			= 8,	// ��
		Transfer	= 9,	// ��������
		Reserve		= 10,	// ������
		Callback	= 11,   // callback
	};

	struct Command
	{
		int			id;			// id ������� (��� �������� ������ � �������)
		std::string sip;		// sip ������������������ �������
		ECommand	command;	// ���� ������� (int)
		int			userId;		// id ������ ��������
	};
	using CommandList = std::vector<Command>;

	struct CommandSendInfoUser 
	{
		std::string	 sip;			// sip ������������������ �������	
		std::string	 ip;			// ip � �������� ������ �������
		unsigned int id;			// id ������������ �� ��
		std::string	 user_login_pc;	// ����� ������� �� �� � �������� ������ �������
		std::string	 pc;			// ��� �� � �������� ��������� ��������
		
		inline bool check() const noexcept
		{
			if (sip.empty() ||
				ip.empty()	||
				id == 0		||
				user_login_pc.empty() ||
				pc.empty())
			{
				return false;
			}

			return true;
		}
	};


	class Status
	{
	public:
		Status();
		~Status();

		void Start();
		void Stop();
		
	private:
		CommandList			m_commandList;
		SP_SQL				m_sql;
		IPotokDispether		m_dispether;
		IFile				m_register;				// ������ ���������� �� �����������\�������������� � ��������
		Log					m_log;

		bool Execute();		
		
		bool IsExistCommand();
		bool GetCommand(std::string &_errorDesciption); // ��������� ����� ������ �� ��

		bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // ���������� �������
		std::string CreateCommand(const Command &_command, const EQueueNumber _queue, const std::string &_rawCommand); // ��������� �������
			
		void DeleteCommand(const Command &_command);	// �������� ����������� ������� 

		void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // �� ������� ���������� �������
		
		bool SendCommand(ECommandType commandType, std::string &_request, std::string &_errorDesciption);	// �������� ������� �� ����������\�������� �� �������
		bool CheckSendingCommand(ECommandType _commandType, std::string &_errorDesciption); // �������� ������� �� ��������� �������

		bool UpdateNewStatus(const Command &_command, std::string &_errorDesciption); // ���������� ������ ������� ���������

		ECommandType GetCommandType(const Command &_command); // ����� ����� ������� ������
		EQueueNumber GetQueueNumber(const ECommand &_command); // ����� ������ �������		
	};
};
using SP_Status = std::shared_ptr<remote::Status>;


//namespace REMOTE_COMMANDS_old {
//		
//	struct R_Commands_old
//	{
//		int			id;				// id ������� (��� �������� ������ � �������)
//		std::string sip;			// sip ������������������ �������
//		remote::ECommand	command;		// ���� ������� (int)
//		std::string ip;				// ip � �������� ������ �������
//		int			user_id;		// id ������������ �� ��
//		std::string user_login_pc;	// ����� ������� �� �� � �������� ������ �������
//		std::string pc;				// ��� �� � �������� ��������� ��������
//	};
//
//	class Remote
//	{
//	public:
//		Remote();
//		~Remote()	= default;		
//
//		unsigned getCountCommand() const;		// ���-�� ������ ������� �� ������ ������ ���� � ������	
//		std::vector<R_Commands_old> list_commads;
//		void startCommand();		// ��������� ������
//
//	private:
//		bool chekNewCommand();		// �������� ���� ����� �������	
//		void createListCommands();	// �������� ������ ������		
//	};
//
//}

#endif // REMOTE_COMMANDS_H
