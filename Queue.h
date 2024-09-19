//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 03.06.2024				//
//	      парсинг звонков попадающих в Очередь		//
//													//	
//////////////////////////////////////////////////////


#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

namespace QUEUE
{
	enum Currentfind
	{
		phone_find,
		waiting_find,
		queue_find,
	};

	struct Pacients
	{
		std::string phone	{ "null" };		// текущий номер телефона который в очереди сейчас
		std::string waiting { "null" };		// время в (сек) которое сейчас в очереди находится
		std::string queue	{ "null" };		// номер очереди
	};

	class Parsing
	{
	public:
		Parsing(const char *fileQueue);
		~Parsing() = default;

		bool isExistList();					// есть ли очередь	
		void show(bool silent = false);
		
		void insertData();					// добавление данных в БД
		bool isExistQueueAfter20hours();	// проверка есть ли не отвеченные записи после 20:00
		void updateQueueAfter20hours();		// обновление данных если звонок пришел того как нет активных операторов на линии	
		bool isExistAnsweredAfter20hours(); // проверка если ли номера по которым закончили разгвоаривать, но не успели обработать
		void updateAnsweredAfter20hours();	// обновление данных когда закончили разговаривать и ушли с линии
		

	private:
		std::string findParsing(std::string str, QUEUE::Currentfind find); // парсинг  
		std::vector<QUEUE::Pacients> pacient_list;			
	};


	struct BD 
	{
		std::string id;
		std::string phone;
		std::string date_time;
		size_t hash{0};
	};

	class QueueBD 
	{
		public:
			QueueBD()	= default;
			~QueueBD()	= default;
			std::vector<QUEUE::BD> list;
	};
}

#endif //QUEUE_H
