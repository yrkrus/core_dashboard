
#ifndef HOUSEKEEPING_H 
#define HOUSEKEEPING_H

#include <string>
#include "IVR.h"


namespace HOUSEKEEPING {
	
	enum class TASKS
	{
		//TaskQueue,
		//TaskLogging,
		//TaskIvr,
		TaskOnHold,
		TaskSmsSending,
	};

	class HouseKeeping
	{
		public:
		HouseKeeping()	= default;
		~HouseKeeping() = default;

		void createTask(TASKS); // создание задачи на выполнение действия
	};
	

	class ExportTable
	{		
	public:

		struct FieldsTable
		{
			int			id{ 0 };
			std::string phone{ "" };
			std::string waiting_time{ "" };
			std::string date_time{ "" };
		};	
			ExportTable()	= default;
			~ExportTable()	= default;

			FieldsTable fileds;
	};

	/*class Queue_old final : public ExportTable
	{
		public:
			Queue_old()  = default;
			~Queue_old() = default;
	
			int			number_queue{ 0 };
			int			sip{ 0 };
			std::string talk_time{ "" };
			int			answered{ 0 };
			int			fail{ 0 };
			size_t		hash{ 0 };
	};*/

	class IVR_ final: public ExportTable
	{
		public:
			IVR_()	= default;			
			~IVR_()	= default;

				
			
			std::string trunk{ "" };
			int			to_queue{ 0 };
			int			to_robot{ 0 };
	};

	class Logging_old final: public ExportTable
	{
		public:
			Logging_old()	= default;
			~Logging_old()	= default;

			std::string		ip{ "0.0.0.0" };
			int				user_id{ 0 };
			std::string		user_login_pc{ "" };
			std::string		pc{""};
			int				action{ 0 };
	};

	class OnHold
	{
	public:
		OnHold() = default;
		~OnHold() = default;

		int				id{ 0 };
		int				sip{ 0 };
		std::string		date_time_start{ "" };
		std::string		date_time_stop{ "" };
		size_t			hash { 0 };

	};

	class SmsSending
	{
	public:
		SmsSending() = default;
		~SmsSending() = default;

		int				id = 0;
		int				user_id = 0;
		std::string		date_time = "";
		std::string		phone = "";
		std::string		message = "";
		size_t			sms_id = 0;
		std::string		status = "";
		std::string		user_login_pc = "";
		int				count_real_sms = 0;
		int				sms_type = -1;
		

	};
}

#endif // !HOUSEKEEPING_H



