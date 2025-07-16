#include "../ISQLConnect.h"

#ifndef ICLEAR_H
#define ICLEAR_H

class IClear 
{
private:
	virtual bool Insert() { return true; }
	virtual void Delete(int _id) = 0;
	virtual bool Get() = 0;
	virtual bool IsExistData() = 0;

protected:	
	SP_SQL		m_sql;

public:
	IClear();
	virtual ~IClear();

	virtual void Execute() = 0;	
};

#endif //ICLEAR_H