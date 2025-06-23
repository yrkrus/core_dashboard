#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef IVR_H
#define IVR_H

static std::string IVR_COMMANDS			= "Playback|lukoil|ivr-3";
static std::string IVR_COMMANDS_EXT1	= "IVREXT";
static std::string IVR_COMMANDS_EXT2	= "Spasibo";
static std::string IVR_REQUEST			= "asterisk -rx \"core show channels verbose\" | grep -E \"" + IVR_COMMANDS + "\" " + " | grep -v \"" + IVR_COMMANDS_EXT1 + "\" " + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" ";


class IVR : public IAsteriskData			
{
public:	
	enum class ecCallerId
	{
		eUnknown = 0,		// неизвестный
		eDomru_220220,		// 220-220
		eDomru_220000,		// 220-000
		eSts,				// STS
		eComagic,			// COMAGIC
		eBeelineMih,		// MIH (михайловка)
	};

	struct IvrCalls
	{
		std::string phone	= "null";					// текущий номер телефона который в IVR слушает
		std::string waiting = "null";					// время в (сек) которое он слушает	
		ecCallerId callerID = ecCallerId::eUnknown;		// откуда пришел звонок		
	};
	
	
	IVR();
	~IVR() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// разбор сырых данных	

private:
	std::vector<IvrCalls>			m_listIvr;	
	SP_SQL							m_sql;


	bool CreateCallers(const std::string&, IvrCalls&);
	bool CheckCallers(const IvrCalls &);												// проверка корреткности стуктуры звонка
	bool IsExistListIvr();
	ecCallerId StringToEnum(const std::string &_str);
	std::string EnumToString(ecCallerId _caller);

	void InsertIvrCalls();																// вставка в БД данных
	void UpdateIvrCalls(unsigned int _id, const IvrCalls &_caller);						// обновдление звонка IVR по БД
	bool IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription);		// есть ли такой номер в БД
	unsigned int GetPhoneIDIvr(const std::string &_phone);								// id phone по БД

};

//template<> TODO сделать на EnumToString


//namespace IVR_OLD 
//{	
//	/*enum Currentfind
//	{
//		phone_find,
//		waiting_find,
//		caller_id,
//	};*/
//
//	/*enum CallerID 
//	{
//		domru_220220,
//		domru_220000,
//		sts,
//		comagic,
//		null_caller,
//		COUNT = 5,
//	*/};
//	
//	//struct Pacients
//	//{
//	//	std::string phone	{"null"};		// текущий номер телефона который в IVR слушает
//	//	std::string waiting {"null"};		// время в (сек) которое он слушает	
//	//	CallerID callerID;					// откуда пришел звонок		
//	//};
//
//	class Parsing 
//	{
//	public:
//		Parsing(const char *fileIVR);
//		~Parsing() = default;			
//	
//		bool isExistList();						// существет ли очередь IVR		
//		//void show(bool silent = false);		
//	//	void insertData();						//добавление данных в БД
//
//	private:	
//		//std::string findParsing(std::string str, IVR_OLD::Currentfind find); // парсинг		
//		//std::vector<IVR_OLD::Pacients> pacient_list;
//
//
//	};
//
//	CallerID getCallerID(std::string str);
//	std::string getCallerID(const CallerID &callerID);
//}




#endif //IVR_H
