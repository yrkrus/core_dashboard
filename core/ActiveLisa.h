/**
 * @file ActiveLisa.h
 * @brief активные звонки Лиза
 * 
 */

 #ifndef ACTIVELISA_H
 #define ACTIVELISA_H

 #include <string>
 #include <vector>
 #include "../different_checks/ICheck.h"
 #include "../interfaces/IFile.h"
 #include "../interfaces/IAsteriskApplication.h"
 #include "../interfaces/ISQLConnect.h"
 #include "../system/Log.h"

// структура звонка
struct ActiveLisaCall
{
	std::string phone;	   // текущий номер телфеона с которым ведется беседа
	std::string phone_raw; // текущий номер телфеона с которым ведется беседа (сырой как по aster проходит)
	int talkTime;		   // время развговора
	ecAsteriskState status = ecAsteriskState::Unknown;
	std::string call_id; // id звонка

	inline bool check() const noexcept
	{
		return ((status != ecAsteriskState::Unknown) &&
				(!phone.empty()) &&
				(!phone_raw.empty()) &&
				(!call_id.empty()));
	}
};
using ActiveLisaCallList = std::vector<ActiveLisaCall>;

struct ActiveLisaCallInBase
{
	int id;
	std::string phone;
	std::string date_time;	
	std::string call_id; // id звонка
	size_t hash;
};
using ActiveLisaCallInBaseList = std::vector<ActiveLisaCallInBase>; 


class ActiveLisa : public ICheck 			
{
public:
    ActiveLisa();
    ~ActiveLisa() override;

    // override ICheck
    virtual bool Execute() override;
    
    
    private:    
    ActiveLisaCallList  		m_activeList;
	ActiveLisaCallInBaseList  	m_activeListBase;
    SP_SQL              		m_sql;
	SP_Log              		m_log;
    IFile						m_rawData;				// список с данными 	


    bool CreateRawData(std::string &_errorDescription);			// получим raw данные
	
	bool CreateCallers(const std::string&, ActiveLisaCall&);
	bool CheckCallers(const ActiveLisaCall&);					// проверка корреткности стуктуры звонка
	
	void FindNewCall();	// поиск новых звонков 
	
	bool IsExistListLisa();
	void InsertLisaCalls();	
	bool IsExistCallLisa(const ActiveLisaCall &_caller, std::string &_errorDescription, bool &_errorConnectSQL);		// есть ли такой номер в БД (queue_lisa)
	
	bool GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id);		// id phone по БД
	void UpdateLisaCalls(uint32_t _id, const ActiveLisaCall &_caller);							// обновдление звонка IVR по БД
	void UpdateCallSuccess();												// находим и обновляем данные когда разговор успешно состоялся 

	bool GetCallInBase(ActiveLisaCallInBaseList&, std::string &_errorDescription); // достать все значения из БД не проверенные(queue_lisa)
	bool IsActiveTalk(const ActiveLisaCallInBase&);	// проверка есть ли такой разговор в активных разговорах

 };

 using SP_ActiveLisa = std::shared_ptr<ActiveLisa>;

 #endif //ACTIVELISA_H


