// *************************************************************
//						 ПУТИ К ФАЙЛАМ
//			  + абстрактный класс получения файла
// *************************************************************

#ifndef IRESPONSEFILEHANDLER_H
#define IRESPONSEFILEHANDLER_H

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <shared_mutex>

enum class ecFilesTypes;


class IResponseFileHandler 
{
public:
	IResponseFileHandler(ecFilesTypes);
	virtual ~IResponseFileHandler();

	bool IsFileExist() const;	

protected:
	std::string			m_fileTXT;	
	std::shared_mutex	m_mutex;			// защита m_file	


	virtual bool GetFile(std::string &_errorDescription)	= 0;
	virtual bool OpenFile(std::string &_errorDescription)	= 0;
	virtual bool ReadFile(std::string &_errorDescription)	= 0;
};

template<typename T>
std::string EnumToString(T _type);

template<>
std::string EnumToString(ecFilesTypes _type);


#endif // IRESPONSEFILEHANDLER_H
