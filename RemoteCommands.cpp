#include "RemoteCommands.h"
#include "SQLRequest.h"
#include "InternalFunction.h"

using namespace INTERNALFUNCTION;

// construct
REMOTE_COMMANDS::Remote::Remote()
{	
	// формируем лист команд при 
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

// создание списка команд
void REMOTE_COMMANDS::Remote::createListCommands()
{
	// если есть новые команды создаем список с текущими командами
	if (!chekNewCommand()) return;

	// формируем лист команд
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		base.createListRemoteCommands(list_commads);
	}
}

// отработка команд
void REMOTE_COMMANDS::Remote::startCommand()
{
	if (!getCountCommand()) return;

	for (const auto &list : list_commads) {
		SQL_REQUEST::SQL base;
		if (base.isConnectedBD())
		{
			base.startRemoteCommand(list.id,list.sip,list.command,list.user_id);

			// проверяем успешно ли выполнили команду
			if (remoteCommandChekedExecution(list.command))
			{
				//SQL_REQUEST::SQL base;
				if (base.isConnectedBD()) 
				{
					// устанавливаем logging
					LOG::Logging log;
					log.createLog(list.command, list.id);

					// обновляем текущий статус оператора					
				
					// новый статус оператора
					REMOTE_COMMANDS::ecStatusOperators status;										
					
					// ВАЖНО!! адекватно работает только if else .. т.к. в кострукции swith case не находится почему то параметр!!!
					if (list.command == LOG::Log::Log_add_queue_5000) {
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_add_queue_5050) {
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_add_queue_5000_5050) {
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_del_queue_5000) {
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					} 
					else if (list.command == LOG::Log::Log_del_queue_5050)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_del_queue_5000_5050)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_available;
					}
					else if (list.command == LOG::Log::Log_home)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_home;
					}
					else if (list.command == LOG::Log::Log_exodus)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_exodus;
					}
					else if (list.command == LOG::Log::Log_break)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_break;
					}
					else if (list.command == LOG::Log::Log_dinner)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_dinner;
					}
					else if (list.command == LOG::Log::Log_postvyzov)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_postvyzov;
					}
					else if (list.command == LOG::Log::Log_studies)	{
						status = REMOTE_COMMANDS::ecStatusOperators::status_studies;
					}
					else if (list.command == LOG::Log::Log_IT)
					{
						status = REMOTE_COMMANDS::ecStatusOperators::status_IT;
					}
					else if (list.command == LOG::Log::Log_transfer)
					{
						status = REMOTE_COMMANDS::ecStatusOperators::status_transfer;
					}
					else if (list.command == LOG::Log::Log_reserve)
					{
						status = REMOTE_COMMANDS::ecStatusOperators::status_reserve;
					}
					else if (list.command == LOG::Log::Log_callback)
					{
						status = REMOTE_COMMANDS::ecStatusOperators::status_callback;
					}
					
					base.updateStatusOperators(list.user_id, status); 

					// удаляем из БД
					base.deleteRemoteCommand(list.id);
				}
			}			
		}
	}
}

// кол-во команд которые на данный момент есть в памяти
unsigned REMOTE_COMMANDS::Remote::getCountCommand() const
{
	return list_commads.size();
}

