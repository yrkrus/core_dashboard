#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

#include <string>
#include "RemoteCommands.h"

namespace utils 
{
	std::string StringFormat(const char *format, ...);
	std::string PhoneParsing(std::string &phone);					// ������� ������ �������� � ���������� ���
	std::string getTalkTime(std::string talk);						// ������� �������� �� ��� -> 00:00:00

	// ������� ������ �� �������� (������ ���-�����-���� 00:00:00 )
	std::string GetCurrentDateTime();							// ������� ����� 
	std::string GetCurrentStartDay();							// ������� ������ ���
	std::string GetCurrentDateTimeAfterMinutes(int minutes);	// ������� ����� - (�����) ��������� ������ 
	//std::string getCurrentDateTimeAfter20hours();				// ������� ����� ����� 20:00 	

	ECommand getRemoteCommand(int command);	// �������������� ������� ��������� ������ �� int -> LOG::Log
	int getRemoteCommand(ECommand command); // �������������� ������� ��������� ������ �� LOG::Log -> int
	int getStatusOperators(EStatus status); // �������������� ������� ��������� ������ �� REMOTE_COMMANDS::StatusOperators -> int

	bool to_bool(const std::string &str);				// ��������� �� std::string -> bool
	std::string to_string(bool value);					// ��������� �� bool -> std::string 
	size_t string_to_size_t(const std::string &str);	// ��������� std::string->size_t

	//@param &_replacmentResponse - ���������� ������
	//@param &_find - �������� ������� ����
	//@param &_repl - �������� �� ������� ����� ��������
	void ReplaceResponseStatus(std::string &_replacmentResponse, const std::string &_find, const std::string &_repl);

} // utils
	
#endif //INTERNALFUNCTION_H