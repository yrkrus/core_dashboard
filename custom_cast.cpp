#include <map>
#include "custom_cast.h"

	


template<>
ecQueueNumber custom_cast::StringToEnum<ecQueueNumber>(const std::string &_str)
{
	if (_str.find("5000") != std::string::npos)				return ecQueueNumber::e5000;
	if (_str.find("5005") != std::string::npos)				return ecQueueNumber::e5005;
	if (_str.find("5050") != std::string::npos)				return ecQueueNumber::e5050;
	if (_str.find("5000 и 5050") != std::string::npos)		return ecQueueNumber::e5000_e5050;

	return ecQueueNumber::eUnknown;
}

template<>
string custom_cast::EnumToString<ecQueueNumber>(ecQueueNumber _queue)
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
string custom_cast::EnumToString<ecCommand>(ecCommand _command)
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
string custom_cast::EnumToString<ecCommandType>(ecCommandType _command)
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
string custom_cast::EnumToString<ecLogType>(ecLogType _type)
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
ecTrunkState custom_cast::StringToEnum<ecTrunkState>(const std::string &_str)
{
	if (_str.find("Unknown") != std::string::npos)		return ecTrunkState::eUnknown;
	if (_str.find("Registered") != std::string::npos)	return ecTrunkState::eRegistered;
	if (_str.find("Request") != std::string::npos)		return ecTrunkState::eRequest;

	return ecTrunkState::eUnknown;	
}

template<>
std::string custom_cast::EnumToString<ecTrunkState>(ecTrunkState _state)
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
std::string custom_cast::EnumToString<ecCallInfoTable>(ecCallInfoTable _table)
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
std::string custom_cast::EnumToString<ecSmsInfoTable>(ecSmsInfoTable _table)
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
ecSmsCode custom_cast::StringToEnum<ecSmsCode>(const std::string &_str)
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


template<>
ecAsteriskApp custom_cast::StringToEnum<ecAsteriskApp>(const std::string &_str)
{
	if (_str.find("AddQueueMember") != std::string::npos)		return ecAsteriskApp::AddQueueMember;
	if (_str.find("ADSIProg") != std::string::npos)				return ecAsteriskApp::ADSIProg;
	if (_str.find("AGI") != std::string::npos)					return ecAsteriskApp::AGI;
	if (_str.find("Answer") != std::string::npos)				return ecAsteriskApp::Answer;
	if (_str.find("AttendedTransfer") != std::string::npos)		return ecAsteriskApp::AttendedTransfer;
	if (_str.find("AudioSocket") != std::string::npos)			return ecAsteriskApp::AudioSocket;
	if (_str.find("Authenticate") != std::string::npos)			return ecAsteriskApp::Authenticate;
	if (_str.find("BackGround") != std::string::npos)			return ecAsteriskApp::BackGround;
	if (_str.find("BackgroundDetect") != std::string::npos)		return ecAsteriskApp::BackgroundDetect;
	if (_str.find("BlindTransfer") != std::string::npos)		return ecAsteriskApp::BlindTransfer;	
	if (_str.find("Bridge") != std::string::npos)		return ecAsteriskApp::Bridge;	
	if (_str.find("BridgeAdd") != std::string::npos)		return ecAsteriskApp::BridgeAdd;	
	if (_str.find("BridgeWait") != std::string::npos)		return ecAsteriskApp::BridgeWait;	
	if (_str.find("Broadcast") != std::string::npos)		return ecAsteriskApp::Broadcast;	
	if (_str.find("Busy") != std::string::npos)		return ecAsteriskApp::Busy;	
	if (_str.find("CallCompletionCancel") != std::string::npos)		return ecAsteriskApp::CallCompletionCancel;	
	if (_str.find("CallCompletionRequest") != std::string::npos)		return ecAsteriskApp::CallCompletionRequest;	
	if (_str.find("CELGenUserEvent") != std::string::npos)		return ecAsteriskApp::CELGenUserEvent;	
	if (_str.find("ChangeMonitor") != std::string::npos)		return ecAsteriskApp::ChangeMonitor;	
	if (_str.find("ChanIsAvail") != std::string::npos)		return ecAsteriskApp::ChanIsAvail;	
	if (_str.find("ChannelRedirect") != std::string::npos)		return ecAsteriskApp::ChannelRedirect;	
	if (_str.find("ChanSpy") != std::string::npos)		return ecAsteriskApp::ChanSpy;	
	if (_str.find("ClearHash") != std::string::npos)		return ecAsteriskApp::ClearHash;	
	if (_str.find("ConfBridge") != std::string::npos)		return ecAsteriskApp::ConfBridge;
	if (_str.find("ConfKick") != std::string::npos)		return ecAsteriskApp::ConfKick;
	if (_str.find("Congestion") != std::string::npos)		return ecAsteriskApp::Congestion;
	if (_str.find("ContinueWhile") != std::string::npos)		return ecAsteriskApp::ContinueWhile;
	if (_str.find("ControlPlayback") != std::string::npos)		return ecAsteriskApp::ControlPlayback;
	if (_str.find("DAHDIRAS") != std::string::npos)		return ecAsteriskApp::DAHDIRAS;
	if (_str.find("DAHDIScan") != std::string::npos)		return ecAsteriskApp::DAHDIScan;
	if (_str.find("DAHDISendCallreroutingFacility") != std::string::npos)		return ecAsteriskApp::DAHDISendCallreroutingFacility;
	if (_str.find("DAHDISendKeypadFacility") != std::string::npos)		return ecAsteriskApp::DAHDISendKeypadFacility;
	if (_str.find("DateTime") != std::string::npos)		return ecAsteriskApp::DateTime;
	if (_str.find("DBdeltree") != std::string::npos)		return ecAsteriskApp::DBdeltree;
	if (_str.find("DeadAGI") != std::string::npos)		return ecAsteriskApp::DeadAGI;
	if (_str.find("Dial") != std::string::npos)		return ecAsteriskApp::Dial;
	if (_str.find("Dictate") != std::string::npos)		return ecAsteriskApp::Dictate;
	if (_str.find("Directory") != std::string::npos)		return ecAsteriskApp::Directory;
	if (_str.find("DISA") != std::string::npos)		return ecAsteriskApp::DISA;
	if (_str.find("DumpChan") != std::string::npos)		return ecAsteriskApp::DumpChan;
	if (_str.find("EAGI") != std::string::npos)		return ecAsteriskApp::EAGI;
	if (_str.find("Echo") != std::string::npos)		return ecAsteriskApp::Echo;
	if (_str.find("ElseIf") != std::string::npos)		return ecAsteriskApp::ElseIf;
	if (_str.find("EndIf") != std::string::npos)		return ecAsteriskApp::EndIf;
	if (_str.find("EndWhile") != std::string::npos)		return ecAsteriskApp::EndWhile;
	if (_str.find("Exec") != std::string::npos)		return ecAsteriskApp::Exec;
	if (_str.find("ExecIf") != std::string::npos)		return ecAsteriskApp::ExecIf;
	if (_str.find("ExecIfTime") != std::string::npos)		return ecAsteriskApp::ExecIfTime;
	if (_str.find("ExitIf") != std::string::npos)		return ecAsteriskApp::ExitIf;
	if (_str.find("ExitWhile") != std::string::npos)		return ecAsteriskApp::ExitWhile;
	if (_str.find("ExtenSpy") != std::string::npos)		return ecAsteriskApp::ExtenSpy;
	if (_str.find("ExternalIVR") != std::string::npos)		return ecAsteriskApp::ExternalIVR;
	if (_str.find("Flash") != std::string::npos)		return ecAsteriskApp::Flash;
	if (_str.find("ForkCDR") != std::string::npos)		return ecAsteriskApp::ForkCDR;
	if (_str.find("GetCPEID") != std::string::npos)		return ecAsteriskApp::GetCPEID;
	if (_str.find("Gosub") != std::string::npos)		return ecAsteriskApp::Gosub;
	if (_str.find("GosubIf") != std::string::npos)		return ecAsteriskApp::GosubIf;
	if (_str.find("Goto") != std::string::npos)		return ecAsteriskApp::Goto;
	if (_str.find("GotoIf") != std::string::npos)		return ecAsteriskApp::GotoIf;
	if (_str.find("GotoIfTime") != std::string::npos)		return ecAsteriskApp::GotoIfTime;
	if (_str.find("Hangup") != std::string::npos)		return ecAsteriskApp::Hangup;
	if (_str.find("HangupCauseClear") != std::string::npos)		return ecAsteriskApp::HangupCauseClear;
	if (_str.find("IAX2Provision") != std::string::npos)		return ecAsteriskApp::IAX2Provision;
	if (_str.find("ICES") != std::string::npos)		return ecAsteriskApp::ICES;
	if (_str.find("If") != std::string::npos)		return ecAsteriskApp::If;
	if (_str.find("ImportVar") != std::string::npos)		return ecAsteriskApp::ImportVar;
	if (_str.find("Incomplete") != std::string::npos)		return ecAsteriskApp::Incomplete;
	if (_str.find("JACK") != std::string::npos)		return ecAsteriskApp::JACK;
	if (_str.find("Log") != std::string::npos)		return ecAsteriskApp::Log;
	if (_str.find("Macro") != std::string::npos)		return ecAsteriskApp::Macro;
	if (_str.find("MacroExclusive") != std::string::npos)		return ecAsteriskApp::MacroExclusive;
	if (_str.find("MacroExit") != std::string::npos)		return ecAsteriskApp::MacroExit;
	if (_str.find("MacroIf") != std::string::npos)		return ecAsteriskApp::MacroIf;
	if (_str.find("MeetMe") != std::string::npos)		return ecAsteriskApp::MeetMe;
	if (_str.find("MeetMeAdmin") != std::string::npos)		return ecAsteriskApp::MeetMeAdmin;
	if (_str.find("MeetMeChannelAdmin") != std::string::npos)		return ecAsteriskApp::MeetMeChannelAdmin;
	if (_str.find("MeetMeCount") != std::string::npos)		return ecAsteriskApp::MeetMeCount;
	if (_str.find("MessageSend") != std::string::npos)		return ecAsteriskApp::MessageSend;
	if (_str.find("Milliwatt") != std::string::npos)		return ecAsteriskApp::Milliwatt;
	if (_str.find("MixMonitor") != std::string::npos)		return ecAsteriskApp::MixMonitor;
	if (_str.find("Monitor") != std::string::npos)		return ecAsteriskApp::Monitor;
	if (_str.find("Morsecode") != std::string::npos)		return ecAsteriskApp::Morsecode;
	if (_str.find("MP3Player") != std::string::npos)		return ecAsteriskApp::MP3Player;
	if (_str.find("MSet") != std::string::npos)		return ecAsteriskApp::MSet;
	if (_str.find("MusicOnHold") != std::string::npos)		return ecAsteriskApp::MusicOnHold;
	if (_str.find("NBScat") != std::string::npos)		return ecAsteriskApp::NBScat;
	if (_str.find("NoCDR") != std::string::npos)		return ecAsteriskApp::NoCDR;
	if (_str.find("NoOp") != std::string::npos)		return ecAsteriskApp::NoOp;
	if (_str.find("ODBC_Commit") != std::string::npos)		return ecAsteriskApp::ODBC_Commit;
	if (_str.find("ODBC_Rollback") != std::string::npos)		return ecAsteriskApp::ODBC_Rollback;
	if (_str.find("ODBCFinish") != std::string::npos)		return ecAsteriskApp::ODBCFinish;
	if (_str.find("Originate") != std::string::npos)		return ecAsteriskApp::Originate;
	if (_str.find("Page") != std::string::npos)		return ecAsteriskApp::Page;
	if (_str.find("Park") != std::string::npos)		return ecAsteriskApp::Park;
	if (_str.find("ParkAndAnnounce") != std::string::npos)		return ecAsteriskApp::ParkAndAnnounce;
	if (_str.find("ParkedCall") != std::string::npos)		return ecAsteriskApp::ParkedCall;
	if (_str.find("PauseMonitor") != std::string::npos)		return ecAsteriskApp::PauseMonitor;
	if (_str.find("PauseQueueMember") != std::string::npos)		return ecAsteriskApp::PauseQueueMember;
	if (_str.find("Pickup") != std::string::npos)		return ecAsteriskApp::Pickup;
	if (_str.find("PickupChan") != std::string::npos)		return ecAsteriskApp::PickupChan;
	if (_str.find("Playback") != std::string::npos)		return ecAsteriskApp::Playback;
	if (_str.find("PlayTones") != std::string::npos)		return ecAsteriskApp::PlayTones;
	if (_str.find("PrivacyManager") != std::string::npos)		return ecAsteriskApp::PrivacyManager;
	if (_str.find("Proceeding") != std::string::npos)		return ecAsteriskApp::Proceeding;
	if (_str.find("Progress") != std::string::npos)		return ecAsteriskApp::Progress;
	if (_str.find("Queue") != std::string::npos)		return ecAsteriskApp::Queue;
	if (_str.find("QueueLog") != std::string::npos)		return ecAsteriskApp::QueueLog;
	if (_str.find("QueueUpdate") != std::string::npos)		return ecAsteriskApp::QueueUpdate;
	if (_str.find("RaiseException") != std::string::npos)		return ecAsteriskApp::RaiseException;
	if (_str.find("Read") != std::string::npos)		return ecAsteriskApp::Read;
	if (_str.find("ReadExten") != std::string::npos)		return ecAsteriskApp::ReadExten;
	if (_str.find("ReceiveFAX") != std::string::npos)		return ecAsteriskApp::ReceiveFAX;
	if (_str.find("ReceiveMF") != std::string::npos)		return ecAsteriskApp::ReceiveMF;
	if (_str.find("ReceiveSF") != std::string::npos)		return ecAsteriskApp::ReceiveSF;
	if (_str.find("ReceiveText") != std::string::npos)		return ecAsteriskApp::ReceiveText;
	if (_str.find("Record") != std::string::npos)		return ecAsteriskApp::Record;
	if (_str.find("Reload") != std::string::npos)		return ecAsteriskApp::Reload;
	if (_str.find("RemoveQueueMember") != std::string::npos)		return ecAsteriskApp::RemoveQueueMember;
	if (_str.find("ResetCDR") != std::string::npos)		return ecAsteriskApp::ResetCDR;
	if (_str.find("RetryDial") != std::string::npos)		return ecAsteriskApp::RetryDial;
	if (_str.find("Return") != std::string::npos)		return ecAsteriskApp::Return;
	if (_str.find("Ringing") != std::string::npos)		return ecAsteriskApp::Ringing;
	if (_str.find("SayAlpha") != std::string::npos)		return ecAsteriskApp::SayAlpha;
	if (_str.find("SayAlphaCase") != std::string::npos)		return ecAsteriskApp::SayAlphaCase;
	if (_str.find("SayDigits") != std::string::npos)		return ecAsteriskApp::SayDigits;
	if (_str.find("SayMoney") != std::string::npos)		return ecAsteriskApp::SayMoney;
	if (_str.find("SayNumber") != std::string::npos)		return ecAsteriskApp::SayNumber;
	if (_str.find("SayOrdinal") != std::string::npos)		return ecAsteriskApp::SayOrdinal;
	if (_str.find("SayPhonetic") != std::string::npos)		return ecAsteriskApp::SayPhonetic;
	if (_str.find("SayUnixTime") != std::string::npos)		return ecAsteriskApp::SayUnixTime;
	if (_str.find("SendDTMF") != std::string::npos)		return ecAsteriskApp::SendDTMF;
	if (_str.find("SendFAX") != std::string::npos)		return ecAsteriskApp::SendFAX;
	if (_str.find("SendImage") != std::string::npos)		return ecAsteriskApp::SendImage;
	if (_str.find("SendMF") != std::string::npos)		return ecAsteriskApp::SendMF;
	if (_str.find("SendSF") != std::string::npos)		return ecAsteriskApp::SendSF;
	if (_str.find("SendText") != std::string::npos)		return ecAsteriskApp::SendText;
	if (_str.find("SendURL") != std::string::npos)		return ecAsteriskApp::SendURL;
	if (_str.find("Set") != std::string::npos)		return ecAsteriskApp::Set;
	if (_str.find("SetAMAFlags") != std::string::npos)		return ecAsteriskApp::SetAMAFlags;
	if (_str.find("Signal") != std::string::npos)		return ecAsteriskApp::Signal;
	if (_str.find("SIPAddHeader") != std::string::npos)		return ecAsteriskApp::SIPAddHeader;
	if (_str.find("SIPDtmfMode") != std::string::npos)		return ecAsteriskApp::SIPDtmfMode;
	if (_str.find("SIPRemoveHeader") != std::string::npos)		return ecAsteriskApp::SIPRemoveHeader;
	if (_str.find("SLAStation") != std::string::npos)		return ecAsteriskApp::SLAStation;
	if (_str.find("SLATrunk") != std::string::npos)		return ecAsteriskApp::SLATrunk;
	if (_str.find("SMS") != std::string::npos)		return ecAsteriskApp::SMS;
	if (_str.find("SoftHangup") != std::string::npos)		return ecAsteriskApp::SoftHangup;
	if (_str.find("SpeechActivateGrammar") != std::string::npos)		return ecAsteriskApp::SpeechActivateGrammar;
	if (_str.find("SpeechBackground") != std::string::npos)		return ecAsteriskApp::SpeechBackground;
	if (_str.find("SpeechCreate") != std::string::npos)		return ecAsteriskApp::SpeechCreate;
	if (_str.find("SpeechDeactivateGrammar") != std::string::npos)		return ecAsteriskApp::SpeechDeactivateGrammar;
	if (_str.find("SpeechDestroy") != std::string::npos)		return ecAsteriskApp::SpeechDestroy;
	if (_str.find("SpeechLoadGrammar") != std::string::npos)		return ecAsteriskApp::SpeechLoadGrammar;
	if (_str.find("SpeechProcessingSound") != std::string::npos)		return ecAsteriskApp::SpeechProcessingSound;
	if (_str.find("SpeechStart") != std::string::npos)		return ecAsteriskApp::SpeechStart;
	if (_str.find("SpeechUnloadGrammar") != std::string::npos)		return ecAsteriskApp::SpeechUnloadGrammar;
	if (_str.find("StackPop") != std::string::npos)		return ecAsteriskApp::StackPop;
	if (_str.find("StartMusicOnHold") != std::string::npos)		return ecAsteriskApp::StartMusicOnHold;
	if (_str.find("Stasis") != std::string::npos)		return ecAsteriskApp::Stasis;
	if (_str.find("StopMixMonitor") != std::string::npos)		return ecAsteriskApp::StopMixMonitor;
	if (_str.find("StopMonitor") != std::string::npos)		return ecAsteriskApp::StopMonitor;
	if (_str.find("StopMusicOnHold") != std::string::npos)		return ecAsteriskApp::StopMusicOnHold;
	if (_str.find("StopPlayTones") != std::string::npos)		return ecAsteriskApp::StopPlayTones;
	if (_str.find("StoreDTMF") != std::string::npos)		return ecAsteriskApp::StoreDTMF;
	if (_str.find("StreamEcho") != std::string::npos)		return ecAsteriskApp::StreamEcho;
	if (_str.find("System") != std::string::npos)		return ecAsteriskApp::System;
	if (_str.find("TestClient") != std::string::npos)		return ecAsteriskApp::TestClient;
	if (_str.find("TestServer") != std::string::npos)		return ecAsteriskApp::TestServer;
	if (_str.find("ToneScan") != std::string::npos)		return ecAsteriskApp::ToneScan;
	if (_str.find("Transfer") != std::string::npos)		return ecAsteriskApp::Transfer;
	if (_str.find("TryExec") != std::string::npos)		return ecAsteriskApp::TryExec;
	if (_str.find("TrySystem") != std::string::npos)		return ecAsteriskApp::TrySystem;
	if (_str.find("UnpauseMonitor") != std::string::npos)		return ecAsteriskApp::UnpauseMonitor;
	if (_str.find("UnpauseQueueMember") != std::string::npos)		return ecAsteriskApp::UnpauseQueueMember;
	if (_str.find("UserEvent") != std::string::npos)		return ecAsteriskApp::UserEvent;
	if (_str.find("Verbose") != std::string::npos)		return ecAsteriskApp::Verbose;
	if (_str.find("VMAuthenticate") != std::string::npos)		return ecAsteriskApp::VMAuthenticate;
	if (_str.find("VMSayName") != std::string::npos)		return ecAsteriskApp::VMSayName;
	if (_str.find("VoiceMail") != std::string::npos)		return ecAsteriskApp::VoiceMail;
	if (_str.find("VoiceMailMain") != std::string::npos)		return ecAsteriskApp::VoiceMailMain;
	if (_str.find("VoiceMailPlayMsg") != std::string::npos)		return ecAsteriskApp::VoiceMailPlayMsg;
	if (_str.find("Wait") != std::string::npos)		return ecAsteriskApp::Wait;
	if (_str.find("WaitDigit") != std::string::npos)		return ecAsteriskApp::WaitDigit;
	if (_str.find("WaitExten") != std::string::npos)		return ecAsteriskApp::WaitExten;
	if (_str.find("WaitForCondition") != std::string::npos)		return ecAsteriskApp::WaitForCondition;
	if (_str.find("WaitForNoise") != std::string::npos)		return ecAsteriskApp::WaitForNoise;
	if (_str.find("WaitForRing") != std::string::npos)		return ecAsteriskApp::WaitForRing;
	if (_str.find("WaitForSignal") != std::string::npos)		return ecAsteriskApp::WaitForSignal;
	if (_str.find("WaitForSilence") != std::string::npos)		return ecAsteriskApp::WaitForSilence;
	if (_str.find("WaitForTone") != std::string::npos)		return ecAsteriskApp::WaitForTone;
	if (_str.find("WaitUntil") != std::string::npos)		return ecAsteriskApp::WaitUntil;
	if (_str.find("While") != std::string::npos)		return ecAsteriskApp::While;
	if (_str.find("Zapateller") != std::string::npos)		return ecAsteriskApp::Zapateller;
	
	return ecAsteriskApp::Unknown;	
}


template<>
ecCallerId custom_cast::StringToEnum<ecCallerId>(const std::string &_str)
{
	if (_str.find("ivr-13") != std::string::npos)			return ecCallerId::Domru_220000;	
	if (_str.find("druOUT_220220") != std::string::npos)	return ecCallerId::Domru_220220;	
	if (_str.find("Dru_220000") != std::string::npos)		return ecCallerId::Domru_220220;
	if (_str.find("sts_") != std::string::npos)				return ecCallerId::Sts;
	if (_str.find("221122") != std::string::npos)			return ecCallerId::Comagic;
	if (_str.find("camaa") != std::string::npos)			return ecCallerId::Comagic;
	if (_str.find("BeeIn") != std::string::npos)			return ecCallerId::BeelineMih;
	
	return ecCallerId::Unknown;
}


template<>
std::string custom_cast::EnumToString<ecCallerId>(ecCallerId _caller)
{
	static std::map<ecCallerId, std::string> callers =
	{
		{ecCallerId::Unknown,		"Unknown"},
		{ecCallerId::Domru_220220,	"220220"},
		{ecCallerId::Domru_220000,	"220000"},
		{ecCallerId::Sts,			"STS"},
		{ecCallerId::Comagic,		"COMAGIC"},
		{ecCallerId::BeelineMih,	"MIH" }
	};

	auto it = callers.find(_caller);
	if (it != callers.end()) 
	{
		return it->second;
	}

	return "Unknown";
}



template<>
ecAsteriskState custom_cast::StringToEnum<ecAsteriskState>(const std::string &_str)
{
	if (_str.find("Down") != std::string::npos)		return ecAsteriskState::Down;	
	if (_str.find("Reserved") != std::string::npos)	return ecAsteriskState::Reserved;	
	if (_str.find("Offhook") != std::string::npos)	return ecAsteriskState::Offhook;
	if (_str.find("Dialing") != std::string::npos)	return ecAsteriskState::Dialing;
	if (_str.find("Ring") != std::string::npos)		return ecAsteriskState::Ring;
	if (_str.find("Ringing") != std::string::npos)	return ecAsteriskState::Ringing;
	if (_str.find("Up") != std::string::npos)		return ecAsteriskState::Up;
	if (_str.find("Busy") != std::string::npos)		return ecAsteriskState::Busy;
	if (_str.find("Unavailable") != std::string::npos)	return ecAsteriskState::Unavailable;
	
	return ecAsteriskState::Unknown;
}


template<>
std::string custom_cast::EnumToString<ecAsteriskState>(ecAsteriskState _state)
{
	static std::map<ecAsteriskState, std::string> states =
	{
		{ecAsteriskState::Down,		"Down"},
		{ecAsteriskState::Reserved,	"Reserved"},
		{ecAsteriskState::Offhook,	"Offhook"},
		{ecAsteriskState::Dialing,			"Dialing"},
		{ecAsteriskState::Ring,		"Ring"},
		{ecAsteriskState::Up,		"Up"},
		{ecAsteriskState::Ringing,	"Ringing" },
		{ecAsteriskState::Busy,	"Busy" },
		{ecAsteriskState::Unavailable,	"Unavailable" }
	
	};

	auto it = states.find(_state);
	if (it != states.end()) 
	{
		return it->second;
	}

	return "Unknown";
}


  