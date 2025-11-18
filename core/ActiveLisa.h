/**
 * @file ActiveLisa.h
 * @brief активные звонки Лиза
 * 
 */

 #ifndef ACTIVELISA_H
 #define ACTIVELISA_H

 #include <string>
 #include <vector>
 #include "../interfaces/IAsteriskData.h"
 #include "../interfaces/IAsteriskApplication.h"
 #include "../interfaces/ISQLConnect.h"
 #include "../system/Log.h"

 

// структура звонка
	struct ActiveLisaCall 
	{
		std::string phone;			// текущий номер телфеона с которым ведется беседа
		std::string phone_raw;		// текущий номер телфеона с которым ведется беседа (сырой как по aster проходит)	
		int talkTime;		        // время развговора  
		ecAsteriskState status = ecAsteriskState::Unknown;
        std::string call_id;			// id звонка		
	
		inline bool check() const noexcept
		{
			return ((status != ecAsteriskState::Unknown) &&
					(!phone.empty()) &&
					(!phone_raw.empty()) &&					
					(!call_id.empty())); 
		}
	
	};
	using ActiveLisaCallList = std::vector<ActiveLisaCall>;


 class ActiveLisa : public IAsteriskData
 {
    public:
    ActiveLisa();
    ~ActiveLisa() override;

     // override IAsteriskData 
    virtual void Start() override;
	virtual void Stop() override;
	virtual void Parsing() override;							// разбор сырых данных	
    
    
    private:    
    ActiveLisaCallList  m_activeList;
    SP_SQL              m_sql;
	SP_Log              m_log;
    
    
    bool CreateCallers(const std::string&, ActiveLisaCall&);
	bool CheckCallers(const ActiveLisaCall&);					// проверка корреткности стуктуры звонка
	bool IsExistListLisa();
	void InsertLisaCalls();	
	bool IsExistCallLisa(const ActiveLisaCall &_caller, std::string &_errorDescription);		// есть ли такой номер в БД (queue_lisa)
	
	bool GetID(const std::string &_phone, const std::string &_call_id, uint32_t &_id);		// id phone по БД
	void UpdateLisaCalls(uint32_t _id, const ActiveLisaCall &_caller);							// обновдление звонка IVR по БД


 };

 using SP_ActiveLisa = std::shared_ptr<ActiveLisa>;

 #endif //ACTIVELISA_H


