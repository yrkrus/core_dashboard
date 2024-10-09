//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 12.08.2024				//
//  ������� �����������\�������������� � �������	//
//													//	
//////////////////////////////////////////////////////
#pragma once
#include "Log.h"
#include <vector>
#include <string>


namespace REMOTE_COMMANDS {

	enum class StatusOperators
	{
		status_available		= 1,        // ��������
		status_home				= 2,        // �����		
		status_exodus			= 3,        // �����
		status_break			= 4,        // �������
		status_dinner			= 5,        // ����
		status_postvyzov		= 6,        // ���������
		status_studies			= 7,        // �����
		status_IT				= 8,        // ��
		status_transfer			= 9,        // ��������
		status_reserve			= 10,		// ������
	};

	
	struct R_Commands
	{
		int			id;						// id ������� (��� �������� ������ � �������)
		std::string sip;			// sip ������������������ �������
		LOG::Log	command;			// ���� ������� (int)
		std::string ip;				// ip � �������� ������ �������
		int			user_id;				// id ������������ �� ��
		std::string user_login_pc;	// ����� ������� �� �� � �������� ������ �������
		std::string pc;				// ��� �� � �������� ��������� ��������
	};

	class Remote
	{
	public:
		Remote();
		~Remote()	= default;		

		unsigned getCountCommand() const;		// ���-�� ������ ������� �� ������ ������ ���� � ������	
		std::vector<R_Commands> list_commads;
		void startCommand();		// ��������� ������

	private:
		bool chekNewCommand();		// �������� ���� ����� �������	
		void createListCommands();	// �������� ������ ������		
	};

}
