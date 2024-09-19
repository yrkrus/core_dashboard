#include "RemoteCommands.h"
#include "SQLRequest.h"
#include "InternalFunction.h"


// construct
REMOTE_COMMANDS::Remote::Remote()
{	
	// ��������� ���� ������ ��� 
	createListCommands();

}

bool REMOTE_COMMANDS::Remote::chekNewCommand()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		return base.remoteCheckNewCommads();
	}
}

// �������� ������ ������
void REMOTE_COMMANDS::Remote::createListCommands()
{
	// ���� ���� ����� ������� ������� ������ � �������� ���������
	if (!chekNewCommand()) return;

	// ��������� ���� ������
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		base.createListRemoteCommands(list_commads);
	}
}

// ��������� ������
void REMOTE_COMMANDS::Remote::startCommand()
{
	if (!getCountCommand()) return;

	for (const auto &list : list_commads) {
		SQL_REQUEST::SQL base;
		if (base.isConnectedBD())
		{
			base.startRemoteCommand(list.id,list.sip,list.command,list.user_id);

			// ��������� ������� �� ��������� �������
			if (remoteCommandChekedExecution(list.command))
			{
				//SQL_REQUEST::SQL base;
				if (base.isConnectedBD()) 
				{
					// ������������� logging
					LOG::Logging log;
					log.createLog(list.command, list.id);

					// ��������� ������� ������ ���������					
				
					// ����� ������ ���������
					REMOTE_COMMANDS::StatusOperators status;										
					
					// �����!! ��������� �������� ������ if else .. �.�. � ���������� swith case �� ��������� ������ �� ��������!!!
					if (list.command == LOG::Log::Log_add_queue_5000) {
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_add_queue_5050) {
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_add_queue_5000_5050) {
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_del_queue_5000) {
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					} 
					else if (list.command == LOG::Log::Log_del_queue_5050)	{
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_del_queue_5000_5050)	{
						status = REMOTE_COMMANDS::StatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_home)	{
						status = REMOTE_COMMANDS::StatusOperators::status_home;
					}
					else if (list.command == LOG::Log::Log_exodus)	{
						status = REMOTE_COMMANDS::StatusOperators::status_exodus;
					}
					else if (list.command == LOG::Log::Log_break)	{
						status = REMOTE_COMMANDS::StatusOperators::status_break;
					}
					else if (list.command == LOG::Log::Log_dinner)	{
						status = REMOTE_COMMANDS::StatusOperators::status_dinner;
					}
					else if (list.command == LOG::Log::Log_postvyzov)	{
						status = REMOTE_COMMANDS::StatusOperators::status_postvyzov;
					}
					else if (list.command == LOG::Log::Log_studies)	{
						status = REMOTE_COMMANDS::StatusOperators::status_studies;
					}
					else if (list.command == LOG::Log::Log_IT)
					{
						status = REMOTE_COMMANDS::StatusOperators::status_IT;
					}
					else if (list.command == LOG::Log::Log_transfer)
					{
						status = REMOTE_COMMANDS::StatusOperators::status_transfer;
					}
					else if (list.command == LOG::Log::Log_reserve)
					{
						status = REMOTE_COMMANDS::StatusOperators::status_reserve;
					}
					
					base.updateStatusOperators(list.user_id, status); 

					// ������� �� ��
					base.deleteRemoteCommand(list.id);
				}
			}			
		}
	}
}

// ���-�� ������ ������� �� ������ ������ ���� � ������
int REMOTE_COMMANDS::Remote::getCountCommand() const
{
	return list_commads.size();
}

