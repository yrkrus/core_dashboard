#include "Log.h"
#include "SQLRequest.h"
#include <vector>


// создание лога
void LOG::Logging::createLog(Log command, int base_id)
{
	SQL_REQUEST::SQL base;

	// добавляем 
	if (base.isConnectedBD())
	{
		base.addLog(command, base_id);
	}
}
