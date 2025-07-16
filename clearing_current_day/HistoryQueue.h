#include <string>
#include <vector>
#include "IClear.h"

#ifndef HISTORYQUEUE_H
#define HISTORYQUEUE_H

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
	};
	using HistoryQueueTable = std::vector<Table>;
	
	
	HistoryQueue();
	~HistoryQueue() override;

	virtual void Execute() override;

private:

	HistoryQueueTable  m_history;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;

};


#endif // HISTORYQUEUE_H