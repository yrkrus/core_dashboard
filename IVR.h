//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 01.06.2024				//
//	      парсинг звонков попадающих в IVR			//
//													//	
//////////////////////////////////////////////////////


#include <string>
#include <vector>
#include <iostream>

#ifndef IVR_H
#define IVR_H

namespace IVR 
{		
	enum Currentfind
	{
		phone_find,
		waiting_find,
		caller_id,
	};

	enum CallerID 
	{
		domru_220220,
		domru_220000,
		sts,
		comagic,
		null_caller,
		COUNT = 5,
	};
	
	struct Pacients
	{
		std::string phone	{"null"};		// текущий номер телефона который в IVR слушает
		std::string waiting {"null"};		// время в (сек) которое он слушает	
		CallerID callerID;					// откуда пришел звонок
		
	};

	class Parsing 
	{
	public:
		Parsing(const char *fileIVR);
		~Parsing() = default;			
	
		bool isExistList();						// существет ли очередь IVR		
		void show(bool silent = false);		
		void insertData();						//добавление данных в БД

	private:	
		std::string findParsing(std::string str, IVR::Currentfind find); // парсинг		
		std::vector<IVR::Pacients> pacient_list;				
	};

	CallerID getCallerID(std::string str);
	std::string getCallerID(const CallerID &callerID);
}


#endif //IVR_H
