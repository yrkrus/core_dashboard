#ifndef HISTORY_LOGGING_H
#define HISTORY_LOGGING_H

#include <string>
#include <vector>
#include "IClear.h"
#include "../Log.h"

class HistoryLogging : public IClear
{

public:

	struct Table
	{
		int			id;		
		std::string ip;
		int			user_id;
		std::string user_login_pc;
		std::string pc;
		std::string	date_time;
		int			action;
	};
	using HistoryLoggingTable = std::vector<Table>;
	

	HistoryLogging();
	~HistoryLogging() override;

	virtual void Execute() override;

private:
	HistoryLoggingTable		m_history;
	Log						m_log;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual int Count() override;
};


#endif // HISTORY_LOGGING_H