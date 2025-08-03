// Базовый класс для запросов к астериску
// Чисто виртуальный

#ifndef IASTERISKDATA_H
#define IASTERISKDATA_H

#include <shared_mutex>
#include <deque>
#include <string>

#include "IFile.h"
#include "PotokDispether.h"

class IAsteriskData
{
private:
	std::string m_name;
	
protected:
	IFile				m_rawData;				// список с данными 
	IPotokDispether		m_dispether;			// отдельный поток который отвечает за получение данных 		

public:
	IAsteriskData(const std::string &_name, unsigned int _timer);
	virtual ~IAsteriskData();

	virtual void Start() = 0;	
	virtual void Stop() = 0;
	virtual void Parsing() = 0;					// разбор сырых данных
	
	std::deque<std::string> GetRawAllData();	// список со списком сырых данных	
	bool IsExistRawData();						// есть ли сырые данные 
	
	std::string GetRawLastData();				// последние с конца сырые данные
	std::string GetRawFirstData();				// первые сырые данные	
	
	void DeleteRawLastData();					// удаление последней сырой записи
	void DeleteRawAllData();					// удаление всех сырых записей
};

#endif // IASTERISKDATA