//  ������� �����������\�������������� � �������	

#include "Log.h"
#include <vector>
#include <string>

#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

static std::string COMMAND_ADD_QUEUE = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" ";
static std::string COMMAND_DEL_QUEUE = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" ";


using namespace LOG_old;

namespace remote
{
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
		int			id;				// id ������� (��� �������� ������ � �������)
		std::string sip;			// sip ������������������ �������
		LOG_old::ecStatus	command;		// ���� ������� (int)
		int			userId;			// id ������ ��������
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
		Logging				m_log;



		bool IsExistCommand();
		bool GetCommand(std::string &_errorDesciption); // ��������� ����� ������ �� ��

		bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // ���������� �������
		void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // �� ������� ���������� �������
	};
};

typedef std::shared_ptr<remote::Status> SP_Status;


namespace REMOTE_COMMANDS_old {

	

	
	struct R_Commands_old
	{
		int			id;				// id ������� (��� �������� ������ � �������)
		std::string sip;			// sip ������������������ �������
		LOG_old::ecStatus	command;		// ���� ������� (int)
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
