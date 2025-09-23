#ifndef IVR_H
#define IVR_H

#include <string>
#include <vector>
#include "IAsteriskData.h"
#include "IAsteriskApplication.h"
#include "ISQLConnect.h"
#include "Log.h"


#define MAX_IVR_PARSING_LINES 9		// максимальное значение при парсинге сырых данных IVR
#define CHANNELS_FIELDS 14			// кол-во полей при разборе начинается разбор с 0 ! итого 14 получиться
#define DELIMITER_CHANNELS_FIELDS '!' // разделитель 

// static std::string IVR_COMMANDS			= "Playback|lukoil|ivr-3";	// ищем только эти слова при формировании IVR
static std::string IVR_COMMANDS		=	"Playback";
// static std::string IVR_COMMANDS_EXT1	= "IVREXT";					// пропуск этой записи
// static std::string IVR_COMMANDS_EXT2	= "Spasibo";				// пропуск этой записи
// static std::string IVR_COMMANDS_EXT3	= "recOfficeOffline";		// пропуск этой записи
// static std::string IVR_COMMANDS_EXT4	= "noservice";				// пропуск этой записи 
// static std::string IVR_COMMANDS_EXT5	= "agent";					// пропуск этой записи 
// static std::string IVR_COMMANDS_EXT6	= "from-internal-xfer";		// пропуск этой записи (перевод звонка)

// static std::string IVR_COMMANDS_IK1 = "rec_IK_AllBusy";				// пропуск этой записи (IVR для ИК отдела)
// static std::string IVR_COMMANDS_IK2 = "rec_IK_Welcome";				// пропуск этой записи (IVR для ИК отдела)
// static std::string IVR_COMMANDS_IK3 = "rec_IK_WorkHours";			// пропуск этой записи (IVR для ИК отдела)

// static std::string IVR_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + IVR_COMMANDS + "\" " 
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT1 + "\" " 
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT3 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT4 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT5 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_EXT6 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK1 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK2 + "\" "
// 																			   + " | grep -v \"" + IVR_COMMANDS_IK3 + "\" ";

static std::string IVR_REQUEST = "asterisk -rx \"core show channels concise\" | grep -E \"" + IVR_COMMANDS + "\" ";

class IVR;
using SP_IVR = std::shared_ptr<IVR>;

enum class ecCallerId
{
	Unknown = 0,		// неизвестный
	Domru_220220,		// 220-220
	Domru_220000,		// 220-000
	Sts,				// STS
	Comagic,			// COMAGIC
	BeelineMih,			// MIH (михайловка)
};

class IVR : public IAsteriskData			
{
public:		

	struct IvrCalls
	{
		std::string	channel;        // имя канала												0
		std::string	context;        // диалплан‐контекст										1
		std::string extension;      // номер(или “s”)											2
		uint16_t priority;	       	// приоритет												3
		ecAsteriskState state = ecAsteriskState::Unknown;		// текущее состояние канала (Up, Ring, Down и т.п.)		
		ecAsteriskApp application = ecAsteriskApp::Unknown;   // текущее приложение(Dial, Playback, …)					5
		std::string	data;           // параметры приложения
		ecCallerId	callerID = ecCallerId::Unknown;       // откуда звонок пришел
		std::string phone;      	// номер телефона 
		std::string	AMAFlags;       // AMA флаги(другие флаги ведения учёта)
		uint16_t duration;          // время жизни канала в секундах
		std::string bridgedChannel; // имя “связного” канала, если есть(иначе пусто)
		uint16_t bridgedDuration;	//время “сращивания”(в секундах)
		std::string uniqueID;       //уникальный идентификатор сессии
		std::string call_id;       //идентификатор “корневого” звонка(call - trace)

		
		//std::string phone;					// текущий номер телефона который в IVR слушает
		//std::string waiting;						// время в (сек) которое он слушает	
	//	ECallerId callerID = Unknown;	// откуда пришел звонок		
		//std::string call_id;						// id звонка

		inline bool check() const noexcept
		{
			// если в phone или waiting есть подстрока "null" 
			// или callerID == Unknown — сразу false
			// if (phone.empty()					||
			// 	waiting.empty()					||
			// 	callerID == ECallerId::Unknown 	||
			// 	call_id.empty()
			// )
			// {
			// 	return false;
			// }
			return true;
		}
	};
	
	
	IVR();
	~IVR() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// разбор сырых данных	

private:
	std::vector<IvrCalls>	m_listIvr;	
	SP_SQL					m_sql;
	Log						m_log;

	bool CreateCallers(const std::string&, IvrCalls&);
	bool CheckCallers(const IvrCalls &);												// проверка корреткности стуктуры звонка
	bool IsExistListIvr();	

	void InsertIvrCalls();																// вставка в БД данных
	void UpdateIvrCalls(int _id, const IvrCalls &_caller);								// обновдление звонка IVR по БД
	bool IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription);		// есть ли такой номер в БД
	int GetPhoneIDIvr(const std::string &_phone);										// id phone по БД

};

#endif //IVR_H
