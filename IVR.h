#include <string>
#include <vector>
#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef IVR_H
#define IVR_H
#define MAX_IVR_PARSING_LINES 9		// максимальное значение при парсинге сырых данных IVR

static std::string IVR_COMMANDS			= "Playback|lukoil|ivr-3";	// ищем только эти слова при формировании IVR
static std::string IVR_COMMANDS_EXT1	= "IVREXT";					// пропуск этой записи
static std::string IVR_COMMANDS_EXT2	= "Spasibo";				// пропуск этой записи
static std::string IVR_COMMANDS_EXT3	= "recOfficeOffline";		// пропуск этой записи
static std::string IVR_COMMANDS_EXT4	= "noservice";				// пропуск этой записи 
static std::string IVR_COMMANDS_EXT5	= "agent";					// пропуск этой записи 
static std::string IVR_COMMANDS_EXT6	= "from-internal-xfer";		// пропуск этой записи (перевод звонка)

static std::string IVR_COMMANDS_IK1 = "rec_IK_AllBusy";				// пропуск этой записи (IVR для ИК отдела)
static std::string IVR_COMMANDS_IK2 = "rec_IK_Welcome";				// пропуск этой записи (IVR для ИК отдела)
static std::string IVR_COMMANDS_IK3 = "rec_IK_WorkHours";			// пропуск этой записи (IVR для ИК отдела)

static std::string IVR_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + IVR_COMMANDS + "\" " 
																			   + " | grep -v \"" + IVR_COMMANDS_EXT1 + "\" " 
																			   + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT3 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT4 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT5 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT6 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK1 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK2 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK3 + "\" ";

class IVR;
using SP_IVR = std::shared_ptr<IVR> ;

class IVR : public IAsteriskData			
{
public:	
	enum class ECallerId
	{
		Unknown = 0,		// неизвестный
		Domru_220220,		// 220-220
		Domru_220000,		// 220-000
		Sts,				// STS
		Comagic,			// COMAGIC
		BeelineMih,			// MIH (михайловка)
	};

	struct IvrCalls
	{
		std::string phone	= "null";					// текущий номер телефона который в IVR слушает
		std::string waiting = "null";					// время в (сек) которое он слушает	
		ECallerId callerID = ECallerId::Unknown;		// откуда пришел звонок		
	
		inline bool check() const noexcept 
		{
			// если в phone или waiting есть подстрока "null" 
			// или callerID == Unknown — сразу false
			if (phone.find("null")		!= std::string::npos	||
				waiting.find("null")	!= std::string::npos	||
				callerID				== ECallerId::Unknown)
			{
				return false;
			}
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


	bool CreateCallers(const std::string&, IvrCalls&);
	bool CheckCallers(const IvrCalls &);												// проверка корреткности стуктуры звонка
	bool IsExistListIvr();
	ECallerId StringToEnum(const std::string &_str);
	std::string EnumToString(ECallerId _caller);

	void InsertIvrCalls();																// вставка в БД данных
	void UpdateIvrCalls(int _id, const IvrCalls &_caller);								// обновдление звонка IVR по БД
	bool IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription);		// есть ли такой номер в БД
	int GetPhoneIDIvr(const std::string &_phone);										// id phone по БД

};

#endif //IVR_H
