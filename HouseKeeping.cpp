#include "HouseKeeping.h"
#include "SQLRequest.h"

void HOUSEKEEPING::HouseKeeping::createTask(TASKS task)
{
	switch (task)
	{
		/*case(TASKS::TaskQueue): {			
			
			SQL_REQUEST::SQL base;
			base.execTaskQueue();
			
			break;
		}*/
		case(TASKS::TaskLogging): {
			
			SQL_REQUEST::SQL base;
			base.execTaskLogging();
			
			break;
		}
		case(TASKS::TaskIvr): {
			
			SQL_REQUEST::SQL base;
			base.execTaskIvr();
			
			break;
		}
		case(TASKS::TaskOnHold):
		{

			SQL_REQUEST::SQL base;
			base.execTaskOnHold();

			break;
		}
		case(TASKS::TaskSmsSending):
		{

			SQL_REQUEST::SQL base;
			base.execTaskSmsSending();

			break;
		}
	}
}
