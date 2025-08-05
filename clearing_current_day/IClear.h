#ifndef ICLEAR_H
#define ICLEAR_H

#include "../ISQLConnect.h"
#include "../Log.h"

enum class ECheckInsert 
{
	No,		// не надо проверять вставку
	Yes,	// надо проверять втсавку
};

class IClear 
{
protected:	
	SP_SQL		m_sql;	
	

public:
	IClear();
	virtual ~IClear();

	virtual bool Execute() = 0;	

	virtual bool Insert() { return true; }
	virtual void Delete(int _id, ECheckInsert _check) = 0;
	virtual bool Get() = 0;
	virtual bool IsExistData() = 0;
	virtual bool CheckInsert(int _id) = 0;
	virtual size_t Count() = 0;
};

#endif //ICLEAR_H