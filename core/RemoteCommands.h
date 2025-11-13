//  команды регистрации\разрегистрации в очереди	

#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

#include <vector>
#include <string>
#include <memory>
#include "../interfaces/IFile.h"
#include "../system/PotokDispether.h"
#include "Queue.h"

static std::string COMMAND_ADD_QUEUE = "asterisk -rx \"queue add member Local/%sip@from-queue/n to %queue penalty 0 as %sip state_interface hint:%sip@ext-local\" ";
static std::string COMMAND_DEL_QUEUE = "asterisk -rx \"queue remove member Local/%sip@from-queue/n from %queue\" ";
static std::string COMMAND_PAUSE_QUEUE = "asterisk -rx \"queue pause member Local/%sip@from-queue/n\"";
									//	  asterisk -rx "queue pause member Local/64197@from-queue/n"

class Log;
//using SP_Log = std::shared_ptr<Log>;

class ISQLConnect;
using SP_SQL = std::shared_ptr<ISQLConnect>;


// удаленные команды (ID команд такие же как и в БД)
enum class ecCommand
{
	Enter				= 0,        // Вход
	Exit				= 1,        // Выход
	AuthError			= 2,        // не успешная авторизация
	ExitForce			= 3,        // Выход (через команду force_closed)
	AddQueue5000		= 4,        // добавление в очередь 5000
	AddQueue5050		= 5,        // добавление в очередь 5050
	AddQueue5000_5050	= 6,        // добавление в очередь 5000 и 5050
	DelQueue5000		= 7,        // удаление из очереди 5000
	DelQueue5050		= 8,        // удаление из очереди 5050
	DelQueue5000_5050	= 9,        // удаление из очереди 5000 и 5050
	Available			= 10,       // доступен
	Home				= 11,       // домой        
	Exodus				= 12,       // исход
	Break				= 13,       // перерыв
	Dinner				= 14,       // обед
	Postvyzov			= 15,       // поствызов
	Studies				= 16,       // учеба
	IT					= 17,       // ИТ
	Transfer			= 18,       // переносы
	Reserve				= 19,       // резерв
	Callback			= 20,       // callback
	AddNewUser			= 21,		// создание нового пользователя
	EditUser			= 22,		// редактирование пользователя
	AddQueue5911		= 23, 		// добавление в очередь 5911
	DelQueue5911		= 24,		// удаление из очереди 5911
};
		
enum class ecCommandType
{
	Unknown = -1,
	Del,		// команда на добавление в очередь
	Add,		// команда на удаление из очереди
	Pause,		// команда на паузу в очереди чтобы звонки новые не поступали
};
	
enum class EStatus
{
	Unknown		= -1,	// неизвестный
	Available	= 1,	// доступен
	Home		= 2,	// домой		
	Exodus		= 3,	// исход
	Break		= 4,	// перерыв
	Dinner		= 5,	// обед
	Postvyzov	= 6,	// поствызов
	Studies		= 7,	// учеба
	It			= 8,	// ИТ
	Transfer	= 9,	// переносы
	Reserve		= 10,	// резерв
	Callback	= 11,   // callback
};

struct Command
{
	int			id;			// id команды (для удобного поиска в запросе)
	std::string sip;		// sip инициализировавший команду
	ecCommand	command;	// сама команда (int)
	int			userId;		// id самого пользака
	bool		delay;		// отложенная команда
	bool 		pause;		// есть ли паузу на прием новых звонков
};
using CommandList = std::vector<Command>;

struct CommandSendInfoUser 
{
	std::string	 sip;			// sip инициализировавший команду	
	std::string	 ip;			// ip с которого пришла команда
	unsigned int id;			// id пользователя по БД
	std::string	 user_login_pc;	// логин зареган на пк с которого пришла команда
	std::string	 pc;			// имя пк с которого отправили коиманду
		
	inline bool check() const noexcept
	{
		if (sip.empty() ||
			ip.empty()	||
			id == 0		||
			user_login_pc.empty() ||
			pc.empty())
		{
			return false;
		}

		return true;
	}
};


class Status
{
public:
	Status();
	~Status();

	void Start();
	void Stop();
		
private:
	CommandList			m_commandList;
	SP_SQL				m_sql;
	IPotokDispether		m_dispether;
	IFile				m_register;				// запрос информации по регистрации\разрегистрации в очередях
	SP_Log				m_log;

	bool Execute();		
		
	bool IsExistCommand();
	bool GetCommand(std::string &_errorDesciption); // получение новых команд из БД

	bool ExecuteCommand(const Command &_command, std::string &_errorDesciption); // выполнение команды
	bool ExecuteCommandPause(const Command &_command, std::string &_errorDesciption); // выполнение команды Pause
	std::string CreateCommand(const Command &_command, const ecQueueNumber _queue, const std::string &_rawCommand); // создвание команды
	std::string CreateCommand(const Command &_command, const std::string &_rawCommand); // создание команды	
	
	void DeleteCommand(const Command &_command);	// удаление выполненной команды 

	void ExecuteCommandFail(const Command &_command, const std::string &_errorStr); // не удачное выполнение команды
		
	bool SendCommand(ecCommandType commandType, std::string &_request, std::string &_errorDesciption);	// отправка команды на добавление\удаление из очереди
	bool CheckSendingCommand(ecCommandType _commandType, std::string &_errorDesciption); // проверка успешно ли выполнена команда

	bool UpdateNewStatus(const Command &_command, std::string &_errorDesciption); // обновление нового статуса оператора

	bool IsTalkOperator(const std::string &_sip, std::string &_errorDesciption); // разговаривает ли оператор или нет
	void CreatePauseQueue(const Command &_command, std::string &_errorDesciption); // ставим на пузу все последующме звонки

	ecCommandType GetCommandType(const Command &_command); // поиск какая команда пришла
	ecQueueNumber GetQueueNumber(const ecCommand &_command); // поиск номера очереди		
};

using SP_Status = std::shared_ptr<Status>;

#endif // REMOTE_COMMANDS_H
