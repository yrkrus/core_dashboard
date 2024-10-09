#include "Queue.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include "InternalFunction.h"
#include "SQLRequest.h"
#include <sstream>

using namespace INTERNALFUNCTION;

// коструктор
QUEUE::Parsing::Parsing(const char *fileQueue)
{
	std::ifstream filequeue;

	filequeue.open(fileQueue);

	if (filequeue.is_open())
	{
		//std::cout << "open file... OK\n";

		std::string line;

		// разберем
		while (std::getline(filequeue, line))
		{
			Pacients pacient;

			pacient.phone	= findParsing(line, QUEUE::Currentfind::phone_find);
			pacient.waiting = findParsing(line, QUEUE::Currentfind::waiting_find);
			pacient.queue	= findParsing(line, QUEUE::Currentfind::queue_find);			

			// добавляем
			if (pacient.phone	!= "null" && 
				pacient.waiting != "null" && 
				pacient.queue	!= "null")
			{
				pacient_list.push_back(pacient);
			}
		}
	}

	filequeue.close();
	
}

// деструткор
//QUEUE::Parsing::~Parsing()
//{
//	if (!pacient_list.empty())
//	{
//		pacient_list.clear();
//	}
//}

// проверка пустой ли список в номерами
bool QUEUE::Parsing::isExistList()
{
	return (pacient_list.empty() ? false : true);
}

void QUEUE::Parsing::show(bool silent)
{
	std::ostringstream buffer;
	
	if (isExistList())
	{
		buffer << "Line QUEUE is (" << pacient_list.size() << ")\n";
		
		if (!silent) {
			buffer << "queue" << "\t    \t" << "phone" << "\t \t" << " wait time" << "\n";

			for (std::vector<QUEUE::Pacients>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
			{
				buffer << it->queue << "\t >> \t" << it->phone << "\t (" << it->waiting << ")\n";
			}
		}	
			
	}
	else
	{
		buffer << "QUEUE is empty!\n";
	}

	 std::cout << buffer.str();
}


//добавление данных в БД
void QUEUE::Parsing::insertData() 
{
	if (this->isExistList())
	{
		SQL_REQUEST::SQL base;

		for (std::vector<QUEUE::Pacients>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
		{
			if (base.isConnectedBD())
			{
				base.insertQUEUE(it->queue.c_str(), it->phone.c_str(), it->waiting.c_str());
			}
		}
		
		
		// находим и обновляем данные если звонок был в очереди, но не дождался ответа от оператора
		if (base.isConnectedBD()) 
		{ 
			base.updateQUEUE_fail(pacient_list);			
		}	

		// находим и обновляем данные когда у нас звонок из IVR попал в очередь
		if (base.isConnectedBD())
		{
			base.updateIVR_to_queue(pacient_list);
		}

		// находим и обновляем данные когда разговор успешно состоялся 
		if (base.isConnectedBD()) 
		{
			base.updateQUEUE_hash(pacient_list);
		}

	}	
}

// проверка есть ли не отвеченные записи после 20:00
bool QUEUE::Parsing::isExistQueueAfter20hours()
{
	SQL_REQUEST::SQL base;

	if (base.isConnectedBD()) {
		return base.isExistQueueAfter20hours();
	}

	return false;
}

// обновление данных если звонок пришел того как нет активных операторов на линии
void QUEUE::Parsing::updateQueueAfter20hours()
{
	SQL_REQUEST::SQL base;

	if (base.isConnectedBD())
	{
		base.updateQUEUE_fail();
	}
}


// проверка если ли номера по которым закончили разгвоаривать, но не успели обработать
bool QUEUE::Parsing::isExistAnsweredAfter20hours()
{
	SQL_REQUEST::SQL base;

	if (base.isConnectedBD())
	{
		return base.isExistAnsweredAfter20hours();
	}

	return false;
}

// обновление данных когда закончили разговаривать и ушли с линии
void QUEUE::Parsing::updateAnsweredAfter20hours()
{
	SQL_REQUEST::SQL base;

	if (base.isConnectedBD())
	{
		base.updateAnswered_fail();
	}
}



// парсинг строки
std::string QUEUE::Parsing::findParsing(std::string str, Currentfind find)
{
	std::vector<std::string> lines;
	std::string current_str;

	bool isNewLine{ false };

	for (size_t i = 0; i != str.length(); ++i)
	{

		if (isNewLine)
		{
			if (!current_str.empty())
			{
				lines.push_back(current_str);
				current_str.clear();
			}
		}

		if (str[i] != ' ') // ищем разделить (разделить пустая строка)
		{
			current_str += str[i];
			isNewLine = false;
		}
		else
		{
			isNewLine = true;
		}
	}

	if (!lines.empty())
	{
		switch (find)
		{
		case QUEUE::Currentfind::phone_find:
		{
			return phoneParsing(lines[7]);	
			break;
		}
		case QUEUE::Currentfind::waiting_find:
		{
			return lines[8];
			break;
		}
		case QUEUE::Currentfind::queue_find: {
			return lines[2];
			break;
		}
		default:
		{
			return "null";
			break;
		}
		}
	}
	else {
		return "null";
	}
}
