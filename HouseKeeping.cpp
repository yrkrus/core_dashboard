#include "HouseKeeping.h"
#include "SQLRequest.h"

void HOUSEKEEPING::HouseKeeping::createTask(TASKS task)
{
	SQL_REQUEST::SQL base;

	switch (task)
	{
		case(TASKS::TaskQueue): {			
			
		base.execTaskQueue();
			break;
		}
		case(TASKS::TaskLogging): {
			
			base.execTaskLogging();
			break;
		}
		case(TASKS::TaskIvr): {
			base.execTaskIvr();
			break;
		}
	}
}
