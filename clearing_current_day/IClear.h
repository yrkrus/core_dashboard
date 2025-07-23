#ifndef ICLEAR_H
#define ICLEAR_H

#include "../ISQLConnect.h"
#include "../Log.h"

enum class ECheckInsert 
{
	No,		// �� ���� ��������� �������
	Yes,	// ���� ��������� �������
};

class IClear 
{
protected:	
	SP_SQL		m_sql;	

public:
	IClear();
	virtual ~IClear();

	virtual void Execute() = 0;	

	virtual bool Insert() { return true; }
	virtual void Delete(int _id, ECheckInsert _check) = 0;
	virtual bool Get() = 0;
	virtual bool IsExistData() = 0;
	virtual bool CheckInsert(int _id) = 0;
	virtual int Count() = 0;
};

#endif //ICLEAR_H