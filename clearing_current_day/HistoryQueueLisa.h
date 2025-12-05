#ifndef HISTORYQUEUELISA_H
#define HISTORYQUEUELISA_H

#include <string>
#include <vector>
#include "IClear.h"

class HistoryQueueLisa : public IClear
{
public:
	
	struct Table 
	{
		int				id;		
		std::string		phone;		
		std::string		date_time;		
		int		        talk_time;
		std::string		call_id;
        bool            to_queue;
        bool            answered;
		size_t			hash;

	};
	using HistoryQueueLisaTable = std::vector<Table>;
	
	
	HistoryQueueLisa();
	~HistoryQueueLisa() override;

	virtual bool Execute() override;

private:

	HistoryQueueLisaTable	m_history;
	SP_Log				    m_log;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual size_t Count() override;

};


#endif // HISTORYQUEUELISA_H