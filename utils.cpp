#include "utils.h"
#include <map>

template<>
EQueueNumber StringToEnum<EQueueNumber>(const std::string &_str)
{
	if (_str.find("5000") != std::string::npos)				return EQueueNumber::e5000;
	if (_str.find("5005") != std::string::npos)				return EQueueNumber::e5005;
	if (_str.find("5050") != std::string::npos)				return EQueueNumber::e5050;
	if (_str.find("5000 � 5050") != std::string::npos)		return EQueueNumber::e5000_e5050;

	return EQueueNumber::Unknown;
}

template<>
string EnumToString<EQueueNumber>(EQueueNumber _queue)
{
	static std::map<EQueueNumber, string> queueNumber =
	{
		{EQueueNumber::Unknown,		"Unknown"},
		{EQueueNumber::e5000,		"5000"},
		{EQueueNumber::e5005,		"5005"},
		{EQueueNumber::e5050,		"5050"},
		{EQueueNumber::e5000_e5050,	"5000 � 5050"},
	};

	auto it = queueNumber.find(_queue);	
	return (it == queueNumber.end()) ? "Unknown" : it->second;
}


template<>
string EnumToString<ECommand>(ECommand _command)
{
	static std::map<ECommand, string> command =
	{
		{ECommand::Enter,				"Enter"},				// ����
		{ECommand::Exit,				"Exit"},				// �����
		{ECommand::AuthError,			"AuthError"},			// �� �������� �����������
		{ECommand::ExitForce,			"ExitForce"},			// ����� (����� ������� force_closed)
		{ECommand::AddQueue5000,		"AddQueue5000"},        // ���������� � ������� 5000
		{ECommand::AddQueue5050,		"AddQueue5050"},        // ���������� � ������� 5050
		{ECommand::AddQueue5000_5050,	"AddQueue5000_5050"},	// ���������� � ������� 5000 � 5050
		{ECommand::DelQueue5000,		"DelQueue500"},         // �������� �� ������� 5000
		{ECommand::DelQueue5050,		"DelQueue5050"},        // �������� �� ������� 5050
		{ECommand::DelQueue5000_5050,	"DelQueue5000_5050"},   // �������� �� ������� 5000 � 5050
		{ECommand::Available,			"Available"},			// ��������
		{ECommand::Home,				"Home"},				// �����        
		{ECommand::Exodus,				"Exodus"},				// �����
		{ECommand::Break,				"Break"},				// �������
		{ECommand::Dinner,				"Dinner"},				// ����
		{ECommand::Postvyzov,			"Postvyzov"},			// ���������
		{ECommand::Studies,				"Studies"},				// �����
		{ECommand::IT,					"IT"},					// ��
		{ECommand::Transfer,			"Transfer"},			// ��������
		{ECommand::Reserve,				"Reserve"},				// ������
		{ECommand::Callback,			"Callback"},			// callback
	};

	auto it = command.find(_command);
	return (it == command.end()) ? "Unknown" : it->second;
}

template<>
string EnumToString<ECommandType>(ECommandType _command)
{
	static std::map<ECommandType, string> command =
	{
		{ECommandType::Add,		"Add"},				
		{ECommandType::Del,		"Del"},
		{ECommandType::Unknown,	"Unknown"},		
	};

	auto it = command.find(_command);
	return (it == command.end()) ? "Unknown" : it->second;
}

