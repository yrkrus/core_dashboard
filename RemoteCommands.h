//  ������� �����������\�������������� � �������	

#include "Log.h"
#include <vector>
#include <string>

#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

static std::string COMMAND_ADD_QUEUE = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" ";
static std::string COMMAND_DEL_QUEUE = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" ";


namespace remote
{
	// ��������� ������� (ID ������ ����� �� ��� � � ��)
	enum class ecCommand
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
	
	
	enum class ecCommandType
	{
		Unknown = -1,
		Del,		// ������� �� ���������� � �������
		Add			// ������� �� �������� �� �������
	};
	
	enum class ecStatusOperator
	{
		ecAvailable = 1,	// ��������
		ecHome		= 2,	// �����		
		ecExodus	= 3,	// �����
		ecBreak		= 4,	// �������
		ecDinner	= 5,	// ����
		ecPostvyzov = 6,	// ���������
		ecStudies	= 7,	// �����
		ecIt		= 8,	// ��
		ecTransfer	= 9,	// ��������
		ecReserve	= 10,	// ������
		ecCallback	= 11,   // callback
	};

	struct Command
	{
		int			id;			// id ������� (��� �������� ������ � �������)
		std::string sip;		// sip ������������������ �������
		ecCommand	command;	// ���� ������� (int)
		int			userId;		// id ������ ��������
	};
	typedef std::vector<Command> CommandList;


	class Status
	{
	public:
		Status();
		~Status();

		void Start();
		void Stop();

		void Execute(); // ���������� ������ TODO ����� ��������� � private
	private:
		CommandList			m_commandList;
		SP_SQL				m_sql;
		IPotokDispether		m_dispether;
		IFile				m_register;			// ������ ���������� �� �����������\�������������� � ��������
		

		bool IsExistCommand();
		bool GetCommand(std::string &_errorDesciption); // ��������� ����� ������ �� ��

		bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // ���������� �������
		bool Add(const Command &_command, std::string &_errorDesciption); // ���������� ������� (����������)
		bool Del(const Command &_command, std::string &_errorDesciption); // ���������� ������� (��������)

		void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // �� ������� ���������� �������
		
		ecCommandType GetCommandType(const Command &_command); // ����� ����� ������� ������
		ecQueueNumber GetQueueNumber(const Command &_command); // ����� ������ �������
	};
};

typedef std::shared_ptr<remote::Status> SP_Status;


namespace REMOTE_COMMANDS_old {
		
	struct R_Commands_old
	{
		int			id;				// id ������� (��� �������� ������ � �������)
		std::string sip;			// sip ������������������ �������
		remote::ecCommand	command;		// ���� ������� (int)
		std::string ip;				// ip � �������� ������ �������
		int			user_id;		// id ������������ �� ��
		std::string user_login_pc;	// ����� ������� �� �� � �������� ������ �������
		std::string pc;				// ��� �� � �������� ��������� ��������
	};

	class Remote
	{
	public:
		Remote();
		~Remote()	= default;		

		unsigned getCountCommand() const;		// ���-�� ������ ������� �� ������ ������ ���� � ������	
		std::vector<R_Commands_old> list_commads;
		void startCommand();		// ��������� ������

	private:
		bool chekNewCommand();		// �������� ���� ����� �������	
		void createListCommands();	// �������� ������ ������		
	};

}

#endif // REMOTE_COMMANDS_H
