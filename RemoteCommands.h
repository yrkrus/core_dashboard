//  команды регистрации\разрегистрации в очереди	

#include "Log.h"
#include <vector>
#include <string>

#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

static std::string COMMAND_ADD_QUEUE = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" ";
static std::string COMMAND_DEL_QUEUE = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" ";


namespace remote
{
	// удаленные команды (ID команд такие же как и в БД)
	enum class ecCommand
	{
		Enter				= 0,         // Вход
		Exit				= 1,         // Выход
		AuthError			= 2,         // не успешная авторизация
		ExitForce			= 3,         // Выход (через команду force_closed)
		AddQueue5000		= 4,         // добавление в очередь 5000
		AddQueue5050		= 5,         // добавление в очередь 5050
		AddQueue5000_5050	= 6,         // добавление в очередь 5000 и 5050
		DelQueue5000		= 7,         // удаление из очереди 5000
		DelQueue5050		= 8,         // удаление из очереди 5050
		DelQueue5000_5050	= 9,         // удаление из очереди 5000 и 5050
		Available			= 10,        // доступен
		Home				= 11,        // домой        
		Exodus				= 12,        // исход
		Break				= 13,        // перерыв
		Dinner				= 14,        // обед
		Postvyzov			= 15,        // поствызов
		Studies				= 16,        // учеба
		IT					= 17,        // ИТ
		Transfer			= 18,        // переносы
		Reserve				= 19,        // резерв
		Callback			= 20,        // callback
	};
	
	
	enum class ecCommandType
	{
		Unknown = -1,
		Del,		// команда на добавление в очередь
		Add			// команда на удаление из очереди
	};
	
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
		int			id;			// id команды (для удобного поиска в запросе)
		std::string sip;		// sip инициализировавший команду
		ecCommand	command;	// сама команда (int)
		int			userId;		// id самого пользака
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
		

		bool IsExistCommand();
		bool GetCommand(std::string &_errorDesciption); // получение новых команд из БД

		bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // выполнение команды
		bool Add(const Command &_command, std::string &_errorDesciption); // выполнение команды (добавление)
		bool Del(const Command &_command, std::string &_errorDesciption); // выполнение команды (удаление)

		void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // не удачное выполнение команды
		
		ecCommandType GetCommandType(const Command &_command); // поиск какая команда пришла
		ecQueueNumber GetQueueNumber(const Command &_command); // поиск номера очереди
	};
};

typedef std::shared_ptr<remote::Status> SP_Status;


namespace REMOTE_COMMANDS_old {
		
	struct R_Commands_old
	{
		int			id;				// id команды (для удобного поиска в запросе)
		std::string sip;			// sip инициализировавший команду
		remote::ecCommand	command;		// сама команда (int)
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
