#ifndef INTERNALFUNCTION_H
#define INTERNALFUNCTION_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>	
#include <sstream>
#include <vector>
#include "../core/RemoteCommands.h"

namespace utils
{
	std::string StringFormat(const char *format, ...);
	std::string PhoneParsing(std::string &phone); // парсинг номера телефона в нормальный вид
	std::string PhoneParsingInternal(std::string &phone); // парсинг номера телефона в нормальный вид
	std::string GetTalkTime(std::string talk);	  // перевод временни из сек -> 00:00:00

	// функции работы со временем (формат год-месяц-день 00:00:00 )
	std::string GetCurrentDateTime();						 // текущее время
	std::string GetCurrentStartDay();						 // текущее начало дня
	std::string GetCurrentDateTimeAfterMinutes(int minutes); // текущее время - (МИНУС) указанная минута

	// std::string getCurrentDateTimeAfter20hours();				// текущее время после 20:00

	ecCommand GetRemoteCommand(int _command); // преобразование текущей удаленной комады из int -> LOG::Log
	int GetRemoteCommand(ecCommand _command); // преобразование текущей удаленной комады из LOG::Log -> int
	int GetStatusOperators(EStatus status);	 // преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int

	bool to_bool(const std::string &str);						// конвертер из std::string -> bool
	std::string to_string(bool value);							// конвертер из bool -> std::string
	size_t string_to_size_t(const std::string &str);			// конвертер std::string->size_t
	time_t string_to_unix_timeshtamp(const std::string &_time); // перевод времени в unixtimestamp

	void Sleep(uint64_t _time);

	//@param &_replacmentResponse - изменяемая строка
	//@param &_find - параметр который ищем
	//@param &_repl - параметр НА которой будем заменять
	void ReplaceResponseStatus(std::string &_replacmentResponse, const std::string &_find, const std::string &_repl);


	// xml create
	boost::property_tree::ptree CreateXML(const std::string &_rawXML);

	// разбор строки с разделителем
	bool SplitDelimiterEntry(const std::string &_lines, std::vector<std::string> &_vLines, const char _delimiter, std::string &_errorDescription); 


} // utils

#endif // INTERNALFUNCTION_H