// Базовый класс для полочуния данных из запроса

#include <string>
#include <deque>
#include <shared_mutex>

#ifndef IFILE_H
#define	IFILE_H

using std::string;

class IFile 
{
private:
		std::deque<string>	m_listRaw;			// сырой список не разобранный (который еще в БД не попал)			
		mutable std::shared_mutex	m_mutex;	// мьютекс для защиты (в момент записи\удаления)
		
		bool GetRequest(const char *_request, string &_response, string &_errorDescription);

public:
	IFile();
	virtual ~IFile();
	
	bool CreateData(string &_request, string &_errorDescription);

	std::deque<string> GetRawAll() const;	// получение всего списка 
	std::string GetRawLast() const;			// получение только последнейй записи
	std::string GetRawFirst() const;		// получение только первой записи

	void DeleteRawLast();					// удаление последней записи
	void DeleteRawAll();					// удаление всех сырых записей
};



#endif // IFILE_H