//  команды регистрации\разрегистрации в очереди	

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
		ecAvailable = 1,	// доступен
		ecHome		= 2,	// домой		
		ecExodus	= 3,	// исход
		ecBreak		= 4,	// перерыв
		ecDinner	= 5,	// обед
		ecPostvyzov = 6,	// поствызов
		ecStudies	= 7,	// учеба
		ecIt		= 8,	// ИТ
		ecTransfer	= 9,	// переносы
		ecReserve	= 10,	// резерв
		ecCallback	= 11,   // callback
	};

	struct Command
	{
		int			id;				// id команды (для удобного поиска в запросе)
		std::string sip;			// sip инициализировавший команду
		LOG_old::ecStatus	command;		// сама команда (int)
		int			userId;			// id самого пользака
	};
	typedef std::vector<Command> CommandList;


	class Status
	{
	public:
		Status();
		~Status();

		void Start();
		void Stop();

		void Execute(); // выполнение команд TODO потом перенести в private
	private:
		CommandList			m_commandList;
		SP_SQL				m_sql;
		IPotokDispether		m_dispether;
		IFile				m_register;			// запрос информации по регистрации\разрегистрации в очередях
		Logging				m_log;



		bool IsExistCommand();
		bool GetCommand(std::string &_errorDesciption); // получение новых команд из БД

		bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // выполнение команды
		void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // не удачное выполнение команды
	};
};

typedef std::shared_ptr<remote::Status> SP_Status;


namespace REMOTE_COMMANDS_old {

	

	
	struct R_Commands_old
	{
		int			id;				// id команды (для удобного поиска в запросе)
		std::string sip;			// sip инициализировавший команду
		LOG_old::ecStatus	command;		// сама команда (int)
		std::string ip;				// ip с которого пришла команда
		int			user_id;		// id пользователя по БД
		std::string user_login_pc;	// логин зареган на пк с которого пришла команда
		std::string pc;				// имя пк с которого отправили коиманду
	};

	class Remote
	{
	public:
		Remote();
		~Remote()	= default;		

		unsigned getCountCommand() const;		// кол-во команд которые на данный момент есть в памяти	
		std::vector<R_Commands_old> list_commads;
		void startCommand();		// отработка команд

	private:
		bool chekNewCommand();		// проверка если новая команда	
		void createListCommands();	// создание списка команд		
	};

}

#endif // REMOTE_COMMANDS_H
