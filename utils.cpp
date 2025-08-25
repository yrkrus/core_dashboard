#include <map>
#include "utils.h"


template<>
ecQueueNumber StringToEnum<ecQueueNumber>(const std::string &_str)
{
	if (_str.find("5000") != std::string::npos)				return ecQueueNumber::e5000;
	if (_str.find("5005") != std::string::npos)				return ecQueueNumber::e5005;
	if (_str.find("5050") != std::string::npos)				return ecQueueNumber::e5050;
	if (_str.find("5000 и 5050") != std::string::npos)		return ecQueueNumber::e5000_e5050;

	return ecQueueNumber::eUnknown;
}

template<>
string EnumToString<ecQueueNumber>(ecQueueNumber _queue)
{
	static std::map<ecQueueNumber, string> queueNumber =
	{
		{ecQueueNumber::eUnknown,	"Unknown"},
		{ecQueueNumber::e5000,		"5000"},
		{ecQueueNumber::e5005,		"5005"},
		{ecQueueNumber::e5050,		"5050"},
		{ecQueueNumber::e5000_e5050,	"5000 и 5050"},
	};

	auto it = queueNumber.find(_queue);	
	return (it == queueNumber.end()) ? "Unknown" : it->second;
}


template<>
string EnumToString<ecCommand>(ecCommand _command)
{
	static std::map<ecCommand, string> command =
	{
		{ecCommand::Enter,				"Enter"},				// Вход
		{ecCommand::Exit,				"Exit"},				// Выход
		{ecCommand::AuthError,			"AuthError"},			// не успешная авторизация
		{ecCommand::ExitForce,			"ExitForce"},			// Выход (через команду force_closed)
		{ecCommand::AddQueue5000,		"AddQueue5000"},        // добавление в очередь 5000
		{ecCommand::AddQueue5050,		"AddQueue5050"},        // добавление в очередь 5050
		{ecCommand::AddQueue5000_5050,	"AddQueue5000_5050"},	// добавление в очередь 5000 и 5050
		{ecCommand::DelQueue5000,		"DelQueue500"},         // удаление из очереди 5000
		{ecCommand::DelQueue5050,		"DelQueue5050"},        // удаление из очереди 5050
		{ecCommand::DelQueue5000_5050,	"DelQueue5000_5050"},   // удаление из очереди 5000 и 5050
		{ecCommand::Available,			"Available"},			// доступен
		{ecCommand::Home,				"Home"},				// домой        
		{ecCommand::Exodus,				"Exodus"},				// исход
		{ecCommand::Break,				"Break"},				// перерыв
		{ecCommand::Dinner,				"Dinner"},				// обед
		{ecCommand::Postvyzov,			"Postvyzov"},			// поствызов
		{ecCommand::Studies,			"Studies"},				// учеба
		{ecCommand::IT,					"IT"},					// ИТ
		{ecCommand::Transfer,			"Transfer"},			// переносы
		{ecCommand::Reserve,			"Reserve"},				// резерв
		{ecCommand::Callback,			"Callback"},			// callback
	};

	auto it = command.find(_command);
	return (it == command.end()) ? "Unknown" : it->second;
}

template<>
string EnumToString<ecCommandType>(ecCommandType _command)
{
	static std::map<ecCommandType, string> command =
	{
		{ecCommandType::Add,		"Add"},				
		{ecCommandType::Del,		"Del"},
		{ecCommandType::Pause,		"Pause"},
		{ecCommandType::Unknown,	"Unknown"},		
	};

	auto it = command.find(_command);
	return (it == command.end()) ? "Unknown" : it->second;
}

template<>
string EnumToString<ecLogType>(ecLogType _type)
{
	static std::map<ecLogType, string> type =
	{
		{ecLogType::eDebug,	"DEBUG"},
		{ecLogType::eError,	"ERROR"},
		{ecLogType::eInfo,	"INFO"},
	};

	auto it = type.find(_type);
	return (it == type.end()) ? "Unknown" : it->second;
}

template<>
ecTrunkState StringToEnum<ecTrunkState>(const std::string &_str)
{
	if (_str.find("Unknown") != std::string::npos)		return ecTrunkState::eUnknown;
	if (_str.find("Registered") != std::string::npos)	return ecTrunkState::eRegistered;
	if (_str.find("Request") != std::string::npos)		return ecTrunkState::eRequest;

	return ecTrunkState::eUnknown;	
}

template<>
std::string EnumToString<ecTrunkState>(ecTrunkState _state)
{
	static std::map<ecTrunkState, string> state =
	{
		{ecTrunkState::eUnknown,		"Unknown"},
		{ecTrunkState::eRegistered,		"Registered"},
		{ecTrunkState::eRequest,		"Request"},
	};

	auto it = state.find(_state);	
	return (it == state.end()) ? "Unknown" : it->second;	
}


template<>
std::string EnumToString<ecCallInfoTable>(ecCallInfoTable _table)
{
	static std::map<ecCallInfoTable, string> table =
	{
		{ecCallInfoTable::eIVR,			"ivr"},
		{ecCallInfoTable::eHistoryIvr,	"history_ivr"},
	};

	auto it = table.find(_table);	
	return (it == table.end()) ? "history_ivr" : it->second;		
}

template<>
std::string EnumToString<ecSmsInfoTable>(ecSmsInfoTable _table)
{
	static std::map<ecSmsInfoTable, string> table =
	{
		{ecSmsInfoTable::eSMS,			"sms_sending"},
		{ecSmsInfoTable::eHistorySMS,	"history_sms_sending"},
	};

	auto it = table.find(_table);	
	return (it == table.end()) ? "history_sms_sending" : it->second;		
}


template<>
ecSmsCode StringToEnum<ecSmsCode>(const std::string &_str)
{
	if (_str.find("queued") != std::string::npos)		return ecSmsCode::eQueued;
	if (_str.find("accepted") != std::string::npos)		return ecSmsCode::eAccepted;
	if (_str.find("delivered") != std::string::npos)	return ecSmsCode::eDelivered;
	if (_str.find("rejected") != std::string::npos)		return ecSmsCode::eRejected;
	if (_str.find("undeliverable") != std::string::npos)	return ecSmsCode::eUndeliverable;
	if (_str.find("error") != std::string::npos)		return ecSmsCode::eError;
	if (_str.find("expired") != std::string::npos)		return ecSmsCode::eExpired;
	if (_str.find("unknown") != std::string::npos)		return ecSmsCode::eUnknown;
	if (_str.find("aborted") != std::string::npos)		return ecSmsCode::eAborted;
	if (_str.find("20107") != std::string::npos)		return ecSmsCode::e20107;	
	if (_str.find("20117") != std::string::npos)		return ecSmsCode::e20117;	
	if (_str.find("20148") != std::string::npos)		return ecSmsCode::e20148;	
	if (_str.find("20154") != std::string::npos)		return ecSmsCode::e20154;	
	if (_str.find("20158") != std::string::npos)		return ecSmsCode::e20158;	
	if (_str.find("20167") != std::string::npos)		return ecSmsCode::e20167;	
	if (_str.find("20170") != std::string::npos)		return ecSmsCode::e20170;	
	if (_str.find("20171") != std::string::npos)		return ecSmsCode::e20171;	
	if (_str.find("20200") != std::string::npos)		return ecSmsCode::e20200;	
	if (_str.find("20202") != std::string::npos)		return ecSmsCode::e20202;	
	if (_str.find("20203") != std::string::npos)		return ecSmsCode::e20203;	
	if (_str.find("20204") != std::string::npos)		return ecSmsCode::e20204;	
	if (_str.find("20207") != std::string::npos)		return ecSmsCode::e20207;	
	if (_str.find("20208") != std::string::npos)		return ecSmsCode::e20208;	
	if (_str.find("20209") != std::string::npos)		return ecSmsCode::e20209;
	if (_str.find("20211") != std::string::npos)		return ecSmsCode::e20211;
	if (_str.find("20212") != std::string::npos)		return ecSmsCode::e20212;
	if (_str.find("20213") != std::string::npos)		return ecSmsCode::e20213;
	if (_str.find("20218") != std::string::npos)		return ecSmsCode::e20218;
	if (_str.find("20230") != std::string::npos)		return ecSmsCode::e20230;
	if (_str.find("20280") != std::string::npos)		return ecSmsCode::e20280;
	if (_str.find("20281") != std::string::npos)		return ecSmsCode::e20281;

	return ecSmsCode::eUnknown;	
}


