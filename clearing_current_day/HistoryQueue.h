#ifndef HISTORYQUEUE_H
#define HISTORYQUEUE_H

#include <string>
#include <vector>
#include "IClear.h"

class HistoryQueue : public IClear
{
public:
	
	struct Table 
	{
		int				id;
		int				number_queue;
		std::string		phone;
		std::string		waiting_time;
		std::string		date_time;		
		int				sip;
		std::string		talk_time;
		int				answered;
		int				fail;
		size_t			hash;
		std::string		call_id;
	};
	using HistoryQueueTable = std::vector<Table>;
	
	
	HistoryQueue();
	~HistoryQueue() override;

	virtual bool Execute() override;

private:

	HistoryQueueTable	m_history;
	Log					m_log;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual size_t Count() override;

};


#endif // HISTORYQUEUE_H