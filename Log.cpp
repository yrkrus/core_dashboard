#include "Log.h"
#include "SQLRequest.h"
#include <vector>


// �������� ����
void LOG::Logging::createLog(Log command, int base_id)
{
	SQL_REQUEST::SQL base;

	// ��������� 
	if (base.isConnectedBD())
	{
		base.addLog(command, base_id);
	}
}
