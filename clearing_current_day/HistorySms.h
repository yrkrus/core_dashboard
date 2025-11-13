#ifndef HISTORY_SMS_H
#define HISTORY_SMS_H

#include <string>
#include <vector>
#include "IClear.h"
#include "../core/SmsInfo.h"

class HistorySms : public IClear
{

public:

	struct Table
	{
		int			id;
		int			user_id;		
		std::string	date_time;
		std::string phone;
		std::string message;
		size_t		sms_id;
		int 		status;
		std::string user_login_pc;
		int			count_real_sms;
		int			sms_type;
		std::string status_date;
		std::string nameOperator;
		std::string nameRegion;
	};
	using HistorySmsTable = std::vector<Table>;	
	
	
	

	HistorySms();
	~HistorySms() override;

	virtual bool Execute() override;

private:
	HistorySmsTable		m_history;
	Log					m_log;
	SMSInfo       		m_smsInfo;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual size_t Count() override;
};


#endif // HISTORY_SMS_H