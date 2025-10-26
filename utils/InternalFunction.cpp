#include "InternalFunction.h"
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

std::string utils::StringFormat(const char *format, ...)
{
	// Создаем объект ostringstream для форматирования строки
	std::ostringstream oss;

	// Инициализируем список аргументов
	va_list args;
	va_start(args, format);

	while (*format)
	{
		if (*format == '%')
		{
			format++;
			switch (*format)
			{
			case 'd': // Для целых чисел
			{
				int i = va_arg(args, int);
				oss << i;
				break;
			}
			case 'u': // беззнаковое целое
			{
				unsigned int u = va_arg(args, unsigned int);
				oss << u;
				break;
			}
			case 'f':
			{ // Для чисел с плавающей точкой
				double d = va_arg(args, double);
				oss << d;
				break;
			}
			case 's':
			{ // Для строк
				char *s = va_arg(args, char *);
				oss << s;
				break;
			}
			default: // Обработка неизвестных спецификаторов
				oss << '%' << *format;
				break;
			}
		}
		else
		{
			oss << *format; // Просто добавляем символ
		}
		format++;
	}

	va_end(args);

	return oss.str();
}

// парсинг номера телефона в нормальный вид
std::string utils::PhoneParsing(std::string &phone)
{
	const auto n = phone.size();

	if (n == 10)
	{
		return "+7" + phone; // 10 цифр → добавляем +7
	}

	if (n == 11)
	{
		return "+" + phone; // 11 цифр → просто + перед номером
	}

	if (n == 12 && phone.rfind("+7", 0) == 0) // 12 символов и начинается с "+7" → уже в норме
	{
		return phone;
	}

	phone.clear();	// всё остальное — «null»
	return phone; 
}

std::string utils::PhoneParsingInternal(std::string &phone)
{
	// // Ищем первое вхождение префикса "SIP/"
	// size_t start_pos = phone.find("SIP/");

	// if (start_pos != std::string::npos)
	// { // Если нашли начало
	// 	// Определяем позицию первого символа после префикса "SIP/"
	// 	start_pos += strlen("SIP/"); // Длина "SIP/" равна 4

	// 	// Извлекаем номер до следующего разделителя "-"
	// 	size_t end_pos = phone.find("-", start_pos); // Ищем "-"

	// 	return phone.substr(start_pos, end_pos - start_pos); // Возвращаем найденный номер
	// }

	if (phone.length() == 5) return phone;

	return "";
}	

// перевод общего кол-ва секунда в 00:00:00 формат
std::string utils::GetTalkTime(std::string talk)
{
	const unsigned int daysCount = 24 * 3600;
	const unsigned short hourCount = 3600;
	const unsigned short minCount = 60;

	std::string resultat;

	int talk_time = std::stoi(talk);

	int days, hour, min, sec;
	std::string shour, smin, ssec;

	days = hour = min = sec = 0;

	days = static_cast<int>(std::floor((talk_time / daysCount)));
	hour = static_cast<int>(std::floor(((talk_time - (days * daysCount)) / hourCount)));
	min = static_cast<int>(std::floor(((talk_time - ((days * daysCount) + (hour * hourCount))) / minCount)));
	sec = static_cast<int>(std::floor((talk_time - ((days * daysCount) + (hour * hourCount) + (min * minCount)))));

	(hour < 10) ? shour = "0" + std::to_string(hour) : shour = std::to_string(hour);
	(min < 10) ? smin = "0" + std::to_string(min) : smin = std::to_string(min);
	(sec < 10) ? ssec = "0" + std::to_string(sec) : ssec = std::to_string(sec);

	resultat = shour + ":" + smin + ":" + ssec;
	return ((days == 0) ? resultat : resultat += std::to_string(days) + "d " + resultat);
}

// текущее время
std::string utils::GetCurrentDateTime()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	// формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1)
	{
		mon = "0" + mon;
	}

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1)
	{
		day = "0" + day;
	}

	std::string hour = std::to_string(now_tm->tm_hour);
	if (hour.length() == 1)
	{
		hour = "0" + hour;
	}

	std::string min = std::to_string(now_tm->tm_min);
	if (min.length() == 1)
	{
		min = "0" + min;
	}

	std::string sec = std::to_string(now_tm->tm_sec);
	if (sec.length() == 1)
	{
		sec = "0" + sec;
	}

	return year + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
}

// текущее начало дня
std::string utils::GetCurrentStartDay()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	struct std::tm *now_tm = std::localtime(&now_c);

	// формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1)
	{
		mon = "0" + mon;
	}

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1)
	{
		day = "0" + day;
	}

	return year + "-" + mon + "-" + day + " 00:00:00";
}

// текущее время - 2 минута
std::string utils::GetCurrentDateTimeAfterMinutes(int minutes)
{
	auto now = std::chrono::system_clock::now();
	auto minute = std::chrono::minutes(minutes);

	std::time_t now_c = std::chrono::system_clock::to_time_t(now - minute);
	struct std::tm *now_tm = std::localtime(&now_c);

	// формат год-месяц-день 00:00:00
	std::string year = std::to_string((now_tm->tm_year + 1900));

	std::string mon = std::to_string((now_tm->tm_mon + 1));
	if (mon.length() == 1)
	{
		mon = "0" + mon;
	}

	std::string day = std::to_string(now_tm->tm_mday);
	if (day.length() == 1)
	{
		day = "0" + day;
	}

	std::string hour = std::to_string(now_tm->tm_hour);
	if (hour.length() == 1)
	{
		hour = "0" + hour;
	}

	std::string min = std::to_string(now_tm->tm_min);
	if (min.length() == 1)
	{
		min = "0" + min;
	}

	std::string sec = std::to_string(now_tm->tm_sec);
	if (sec.length() == 1)
	{
		sec = "0" + sec;
	}

	return year + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
}

	// текущее время после 20:00
	// std::string utils::getCurrentDateTimeAfter20hours()
	//{
	//	auto now = std::chrono::system_clock::now();
	//	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	//	struct std::tm *now_tm = std::localtime(&now_c);
	//
	//	//формат год-месяц-день 00:00:00
	//	std::string year = std::to_string((now_tm->tm_year + 1900));
	//
	//	std::string mon = std::to_string((now_tm->tm_mon + 1));
	//	if (mon.length() == 1) { mon = "0" + mon; }
	//
	//	std::string day = std::to_string(now_tm->tm_mday);
	//	if (day.length() == 1) { day = "0" + day; }
	//
	//	return year + "-" + mon + "-" + day + " 20:00:00";
	//}

// преобразование текущей удаленной комады из int -> REMOTE_COMMANDS::Command
ecCommand utils::GetRemoteCommand(int _command) // TODO переделать на template IntegerToEnum
{
	return static_cast<ecCommand>(_command);
}

// преобразование текущей удаленной комады из LOG::Log -> int
int utils::GetRemoteCommand(ecCommand _command) // TODO переделать на template EnumToInteger
{
	return static_cast<int>(_command);
}

// преобразование текущей удаленной комады из REMOTE_COMMANDS::StatusOperators -> int
int utils::GetStatusOperators(EStatus status) // TODO переделать на template EnumToInteger
{
	return static_cast<int>(status);
}

bool utils::to_bool(const std::string &str)
{
	return ((str == "true") ? true : false);
}

std::string utils::to_string(bool value)
{
	return (value ? "true" : "false");
}

size_t utils::string_to_size_t(const std::string &str)
{
	std::stringstream stream(str);
	size_t output;
	stream >> output;

	return output;
}

time_t utils::string_to_unix_timeshtamp(const std::string &_time)
{
	std::tm tm{};
	std::istringstream ss(_time);

	// Важно: Используйте std::get_time для корректного парсинга
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

	if (ss.fail())
	{
		// некорректный формат даты
		return -1;
	}

	return mktime(&tm);
}

void utils::Sleep(uint64_t _time)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(_time));
}

void utils::ReplaceResponseStatus(std::string & _replacmentResponse, const std::string &_find, const std::string &_repl)
{
	// замена для номера очереди
	if (_find.find("%queue") != std::string::npos)
	{
		size_t position = _replacmentResponse.find(_find);
		_replacmentResponse.replace(position, _find.length(), _repl);
	}

	// замена для sip
	if (_find.find("%sip") != std::string::npos)
	{
		size_t position = _replacmentResponse.find(_find);
		while (position != std::string::npos)
		{
			_replacmentResponse.replace(position, _find.length(), _repl);
			position = _replacmentResponse.find(_find);
		}
	}

	if (_find.find("%user") != std::string::npos)
	{
		size_t position = _replacmentResponse.find(_find);
		_replacmentResponse.replace(position, _find.length(), _repl);
	}

	if (_find.find("%pass") != std::string::npos)
	{
		size_t position = _replacmentResponse.find(_find);
		_replacmentResponse.replace(position, _find.length(), _repl);
	}

	if (_find.find("%sms_id") != std::string::npos)
	{
		size_t position = _replacmentResponse.find(_find);
		while (position != std::string::npos)
		{
			_replacmentResponse.replace(position, _find.length(), _repl);
			position = _replacmentResponse.find(_find);
		}
	}
}

boost::property_tree::ptree utils::CreateXML(const std::string &_rawXML)
{
	std::istringstream iss(_rawXML);
	boost::property_tree::ptree xml;
	boost::property_tree::read_xml(iss, xml);

	return xml;
}

// разбор строки с разделителем
bool utils::SplitDelimiterEntry(const std::string &_lines, std::vector<std::string> &_vLines, const char _delimiter, std::string &_errorDescription)
{
	_errorDescription.clear();

	if (_lines.empty())
	{
		_errorDescription = "empty _lines";
		return false;
	}

	_vLines.clear();

	std::string field;
	for (char c : _lines)
	{
		if (c == _delimiter)
		{
			_vLines.push_back(field);
			field.clear();
		}
		else
		{
			field.push_back(c);
		}
	}

	_vLines.push_back(field);

	return true;
}
