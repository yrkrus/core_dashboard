#ifndef IVR_H
#define IVR_H

#include <string>
#include <vector>
#include "../interfaces/IAsteriskData.h"
#include "../interfaces/IAsteriskApplication.h"
#include "../interfaces/ISQLConnect.h"
#include "../system/Log.h"
#include "Queue.h"


class IVR;
using SP_IVR = std::shared_ptr<IVR>;

enum class ecCallerId
{
	Unknown = 0,		// неизвестный
	Domru_220220,		// 220-220
	Domru_220000,		// 220-000
	Sts,				// STS
	Comagic,			// COMAGIC
	BeelineMih,			// MIH (михайловка)`// TODO удалить не используется
	InternalCaller,		// внутренний звонок // TODO для звонокв в очередь 5911 (техподдержка ИК)
};

// вспомогательный класс для времени в ivr
class IVRTime
{
	private:
	SP_SQL								m_sql;
	std::map<ecQueueNumber, uint8_t> 	m_value; 
	
	void CreateMap(ecQueueNumber);	
	uint16_t GetTime(ecQueueNumber);

	public:
	IVRTime();
	~IVRTime();

	uint16_t time(ecQueueNumber);
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
		ecAsteriskApp application = ecAsteriskApp::Unknown;   	// текущее приложение(Dial, Playback, …)					5
		std::string	data;           // параметры приложения
		ecCallerId	callerID = ecCallerId::Unknown;       // откуда звонок пришел
		std::string phone;      	// номер телефона 
		std::string	AMAFlags;       // AMA флаги(другие флаги ведения учёта)
		uint16_t duration;          // время жизни канала в секундах
		std::string bridgedChannel; // имя “связного” канала, если есть(иначе пусто)
		uint16_t bridgedDuration;	// время “сращивания”(в секундах)
		std::string uniqueID;       // уникальный идентификатор сессии
		std::string call_id;        // идентификатор “корневого” звонка(call - trace)	id звонка
		ecQueueNumber queue = ecQueueNumber::eUnknown; // в какую очередь зайдет звонок

		inline bool check() const noexcept
		{
			return ((application == ecAsteriskApp::Playback) &&
					(callerID != ecCallerId::Unknown) &&
					(!phone.empty()) &&
					(!call_id.empty()) &&
					(bridgedDuration > 0) &&
					(queue != ecQueueNumber::eUnknown)); 
		}
	};
	
	
	IVR();
	~IVR() override;

	 // override IAsteriskData 
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Parsing() override;							// разбор сырых данных	

private:
	std::vector<IvrCalls>	m_listIvr;	
	SP_SQL					m_sql;
	Log						m_log;
	IVRTime					m_time;

	bool CreateCallers(const std::string&, IvrCalls&);
	bool CheckCallers(const IvrCalls &);												// проверка корреткности стуктуры звонка
	bool IsExistListIvr();	
	void InsertIvrCalls();																// вставка в БД данных
	void UpdateIvrCalls(uint32_t _id, const IvrCalls &_caller);							// обновдление звонка IVR по БД
	bool IsExistCallIvr(const IvrCalls &_caller, std::string &_errorDescription);		// есть ли такой номер в БД (ivr)

	void IvrLoop(const IvrCalls &_caller); 		// звонок кидаем в ivr_loop (т.к. он повторный)
	bool IsExistCallIvrLoop(const IvrCalls &_caller);// есть ли такой номер в БД (ivr_loop)	
	void InsertIvrLoop(const IvrCalls &_caller);
	void UpdateIvrLoop(const IvrCalls &_caller, int _id);
	bool GetIDLoop(const std::string &_phone, const std::string &_call_id, uint32_t &_id);	// id phone по БД (ivr_loop)
	

	bool GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id);	// id phone по БД
	
	ecQueueNumber FindQueue(ecCallerId _caller);										// получить номер очереди из ecCallerId
};

#endif //IVR_H
