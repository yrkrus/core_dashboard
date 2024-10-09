//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 12.08.2024				//
//  команды регистрации\разрегистрации в чоереди	//
//													//	
//////////////////////////////////////////////////////
#pragma once
#include "Log.h"
#include <vector>
#include <string>


namespace REMOTE_COMMANDS {

	enum class StatusOperators
	{
		status_available		= 1,        // доступен
		status_home				= 2,        // домой		
		status_exodus			= 3,        // исход
		status_break			= 4,        // перерыв
		status_dinner			= 5,        // обед
		status_postvyzov		= 6,        // поствызов
		status_studies			= 7,        // учеба
		status_IT				= 8,        // ИТ
		status_transfer			= 9,        // переносы
		status_reserve			= 10,		// резерв
	};

	
	struct R_Commands
	{
		int			id;						// id команды (для удобного поиска в запросе)
		std::string sip;			// sip инициализировавший команду
		LOG::Log	command;			// сама команда (int)
		std::string ip;				// ip с которого пришла команда
		int			user_id;				// id пользователя по БД
		std::string user_login_pc;	// логин зареган на пк с которого пришла команда
		std::string pc;				// имя пк с которого отправили коиманду
	};

	class Remote
	{
	public:
		Remote();
		~Remote()	= default;		

		unsigned getCountCommand() const;		// кол-во команд которые на данный момент есть в памяти	
		std::vector<R_Commands> list_commads;
		void startCommand();		// отработка команд

	private:
		bool chekNewCommand();		// проверка если новая команда	
		void createListCommands();	// создание списка команд		
	};

}
