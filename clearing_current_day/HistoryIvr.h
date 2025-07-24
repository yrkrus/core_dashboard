#ifndef HISTORY_IVR_H
#define HISTORY_IVR_H

#include <string>
#include <vector>
#include "IClear.h"

class HistoryIvr : public IClear
{

public:

	struct Table
	{
		int			id;
		std::string phone;
		std::string waiting_time;
		std::string date_time;
		std::string trunk;
		int			to_queue;
		int			to_robot;
	};
	using HistoryIvrTable = std::vector<Table>;


	HistoryIvr();
	~HistoryIvr() override;

	virtual void Execute() override;

private:
	HistoryIvrTable  m_history;
	Log				 m_log;

	virtual bool Insert(const Table &_field, std::string &_errorDescription);
	virtual void Delete(int _id, ECheckInsert _check) override;
	virtual bool Get() override;
	virtual bool IsExistData() override;
	virtual bool CheckInsert(int _id) override;
	virtual int Count() override;	
};


#endif // HISTORY_IVR_H