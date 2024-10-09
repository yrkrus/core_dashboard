#include "ActiveSip.h"
#include "InternalFunction.h"
#include "Constants.h"
#include "SQLRequest.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <sstream>

using namespace INTERNALFUNCTION;

// коструктор
ACTIVE_SIP::Parsing::Parsing(const char *fileActiveSip)
{
	// найдем активных операторов в линии
	createListActiveOperators();
	
	// есть ли активные SIP операторы в линии 
	if (!isExistListActiveOperators()) {
		std::cout << "\nActive SIP operators is empty!\n";
		return;
	}

	std::ifstream sip;

	sip.open(fileActiveSip);

	if (sip.is_open())
	{
		// разберем  ТУТ ПОКА в 1 потоке все происходит потом сделать многопочную!!! 
		if (!list_operators.empty()) {		
			
			std::string line;
			while (std::getline(sip, line))
			{	
				for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {					
				
					if (line.find("Local/" + it->sip_number)	!= std::string::npos) {
						if (line.find("Ring")					== std::string::npos) {
							if (line.find("Down")				== std::string::npos) {
								if (line.find("Outgoing")		== std::string::npos) {

									Pacients pacient;

									pacient.internal_sip = findParsing(line, ACTIVE_SIP::Currentfind::internal_sip_find, it->sip_number);
									pacient.phone		 = findParsing(line, ACTIVE_SIP::Currentfind::phone_find, it->sip_number);
									pacient.talk_time	 = findParsing(line, ACTIVE_SIP::Currentfind::talk_time_find, it->sip_number);


									// добавляем
									if (pacient.internal_sip != "null" &&
										pacient.phone		 != "null" &&
										pacient.talk_time	 != "null")
									{
										active_sip_list.push_back(pacient);
										break; // нет смысла дальше while т.к. нашли нужные данные
									}
								}
							}						
						}			
					}					
				}				
			}
		}		
	}

	sip.close();	
}

// деструткор
ACTIVE_SIP::Parsing::~Parsing()
{
	if (!active_sip_list.empty()) { active_sip_list.clear(); }
}

// проверка пустой ли список в номерами
bool ACTIVE_SIP::Parsing::isExistList()
{
	return (active_sip_list.empty() ? false : true);
}

// получаем активный лист операторов которые у нас сейчас в линии находятся
void ACTIVE_SIP::Parsing::createListActiveOperators()
{		
	for (size_t i = 0; i != CONSTANTS::AsteriskQueue::COUNT; ++i) {
	
		std::string responce = CONSTANTS::cActiveSipOperatorsResponse;

		// заменяем %queue на номер очереди
		std::string repl = "%queue";
		size_t position = responce.find(repl);
		responce.replace(position, repl.length(), getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));

		if (!CONSTANTS::DEBUG_MODE) {
			system(responce.c_str());
		}			

		findActiveOperators(CONSTANTS::cActiveSipOperatorsName.c_str(), getNumberQueue(static_cast<CONSTANTS::AsteriskQueue>(i)));
	}

	// добавим\обновим очереди 
	insert_updateQueueNumberOperators();

}


void ACTIVE_SIP::Parsing::show(bool silent)
{
	std::ostringstream buffer;
	
	if (this->isExistList()) {
		buffer << "Line Active SIP is (" << active_sip_list.size() << ")\n";
		
		if (!silent) {
			buffer << "sip" << "\t    \t" << "phone" << "\t \t" << " talk time" << "\n";

			for (const auto &list : active_sip_list)
			{
				if (!getSipIsOnHold(list.internal_sip)) {
					buffer << list.internal_sip << "\t >> \t" << list.phone << "\t (" << getTalkTime(list.talk_time) << ")\n";
				}
				else {
					buffer << list.internal_sip << " (OnHold) \t" << list.phone << "\t (" << getTalkTime(list.talk_time) << ")\n";
				}				
			}
		}		
	}	
	else {
		buffer << "Active SIP is empty!\n";
	}

	std::cout << buffer.str();
}

// парсинг строки
std::string ACTIVE_SIP::Parsing::findParsing(std::string str, ACTIVE_SIP::Currentfind find, const std::string number_operator)
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

		if (str[i] != '!') // ищем разделить (разделить !)
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
	  // защита от sigmentation fault!
		if (lines.size() < 10) {
			return "null";
		}
		
		switch (find)
		{
			case ACTIVE_SIP::Currentfind::phone_find:
			{
				return phoneParsing(lines[7]);
				break;
			}
			case ACTIVE_SIP::Currentfind::internal_sip_find:	// внутренний номер SIP (мы его и так знаем из функции)
			{
				return number_operator;
				break;
			}
			case ACTIVE_SIP::Currentfind::talk_time_find:		// время разговора
			{
				return lines[9];
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


// парсинг нахождения активного sip оператора
std::string ACTIVE_SIP::Parsing::findNumberSip(std::string &str)
{
	// 6 т.к. lenght("Local/) = 6	
	return str.substr( str.find_first_of("Local/") + 6,  str.find_first_of("@") - str.find_first_of("Local/") - 6);	
}


bool ACTIVE_SIP::Parsing::findOnHold(std::string &str)
{
	return ((str.find("On Hold") != std::string::npos) ? true : false);	
}


// // парсинг #2 (для activeoperaots)
void ACTIVE_SIP::Parsing::findActiveOperators(const char *fileOperators, std::string queue)
{
	
	std::ifstream file;
	file.open(fileOperators);

	if (!file.is_open()) {
		// не получается открыть файл уходим из функции
		return;		
	}

	std::string line;

	// разберем
	while (std::getline(file, line))
	{
		// заносить активные CAllers не требуется
		if (line.find("Callers") != std::string::npos) {
			break;
		}
		
		// проверим есть ли активный sip
		if (line.find("Local/") != std::string::npos) {
			// найдем активный sip
			
			Operators active_operator;
			active_operator.sip_number = findNumberSip(line);
			active_operator.queue.push_back(queue);
			active_operator.isOnHold = findOnHold(line);
			
			// проверим есть ли уже такой sip чтобы добавить ему только очередь
			bool isExistOperator{ false };
			for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it) {
				
				if (it->sip_number == active_operator.sip_number) {
					isExistOperator = true;
					it->queue.push_back(queue);
					break;
				}
			}
			
			if (!isExistOperator) {
				list_operators.push_back(active_operator);
			}			
		}
	}

	file.close();
}


// добавление номена очереди оператора	
void ACTIVE_SIP::Parsing::insert_updateQueueNumberOperators()
{
	if (!isExistListActiveOperators())	{			
		// очищаем номера очереди операторов
		if (isExistQueueOperators()) {
			clearQueueNumberOperators();
			return;
		}			
	} 	

	SQL_REQUEST::SQL base;	

	// проверим вдруг из очереди оператор убежал, тогда надо удалить sip из БД
	if (base.isConnectedBD()) {
		base.checkOperatorsQueue(list_operators);
	}		


	// добавляем в БД
	for (std::vector<Operators>::iterator it = list_operators.begin(); it != list_operators.end(); ++it)
	{
		if (base.isConnectedBD())
		{			
			// вдруг очередь 5000 и 5050
			for (size_t i = 0; i != it->queue.size(); ++i) {
				// проверим есть ли такая запись
				if (!base.isExistOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str())) {
					
					//записи нет добавляем
					base.insertOperatorsQueue(it->sip_number.c_str(), it->queue[i].c_str());

				}				
			}			
		}
	}
}

// есть ли операторы в очередях
bool ACTIVE_SIP::Parsing::isExistQueueOperators()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		return base.isExistOperatorsQueue();
	}

	return false;
}



// очистка номеров очереди операторов
void ACTIVE_SIP::Parsing::clearQueueNumberOperators()
{
	SQL_REQUEST::SQL base;
	if (base.isConnectedBD())
	{
		base.clearOperatorsQueue();
	}
}

void ACTIVE_SIP::Parsing::clearListOperatorsPhoneOnHold()
{
	for (auto &list : list_operators) {
		list.phoneOnHold = "null";
	}
}

bool ACTIVE_SIP::Parsing::getSipIsOnHold(std::string sip)
{
	for (auto &list : list_operators) {
		if ( list.sip_number == sip) {			
			return (list.isOnHold ? true : false);			
		}
	}
}


bool ACTIVE_SIP::Parsing::isExistListActiveOperators()
{
	return (!list_operators.empty() ? true : false );
}


//добавление данных в БД
void ACTIVE_SIP::Parsing::updateData()
{
	if (this->isExistList())
	{			
		
		for (std::vector<ACTIVE_SIP::Pacients>::iterator it = active_sip_list.begin(); it != active_sip_list.end(); ++it)
		{
			SQL_REQUEST::SQL base;
			if (base.isConnectedBD())
			{
				base.updateQUEUE_SIP(it->phone.c_str(),it->internal_sip.c_str(),it->talk_time.c_str());				
			}
		}
		
		// обновление данных по операторам находящимся в статусе onHold
	//	SQL_REQUEST::SQL base;
	//	if (base.isConnectedBD()) {
	//		base.updateOperatorsOnHold(this);
	//	}			

	}
	/*else {
		typedef std::vector<ACTIVE_SIP::OnHold> operators_onhold;
		
		// найдем все sip операторы которые числяться по БД в статусе onHold	
		SQL_REQUEST::SQL base;
		auto onHold = base.createOnHoldSip();

		if (!onHold->empty()) {
			// очищаем список т.к. операторов нет аквтиных
			for (operators_onhold::iterator operators_hold = onHold->begin(); operators_hold != onHold->end(); ++operators_hold)
			{
				SQL_REQUEST::SQL base;
				base.updateOperatorsOnHold(operators_hold->id);
			}
		}

		delete onHold;
	}*/
}

std::vector<ACTIVE_SIP::Operators> ACTIVE_SIP::Parsing::getListOperators()
{
	// очищаем лист phoneOnHold
	clearListOperatorsPhoneOnHold();	
	
	// обновим данные по phoneOnHold
	for (auto &operators : list_operators) {
		if (operators.isOnHold) {
				
			for (auto &list_active : active_sip_list)
			{
				if (list_active.internal_sip == operators.sip_number) {
					operators.phoneOnHold = list_active.phone;
					//break;
				}
			}
		}
	}	
	
	return this->list_operators;
}


