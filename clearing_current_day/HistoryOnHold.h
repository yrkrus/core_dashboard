#ifndef HISTORY_ONHOLD_H
#define HISTORY_ONHOLD_H

#include <string>
#include <vector>
#include "IClear.h"

class HistoryOnHold : public IClear
{

public:

	struct Table
	{
		int			id;
		int			sip;
		std::string	date_time_start;		
		std::string date_time_stop;
		std::string phone;
	};
	using HistoryOnHoldTable = std::vector<Table>;

	HistoryOnHold();
	~HistoryOnHold() override;

	virtual bool Execute() override;

private:
	HistoryOnHoldTable		m_history;
	SP_Log					m_log;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual size_t Count() override;
};


#endif // HISTORY_ONHOLD_H