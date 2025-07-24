// *************************************************************
//			 Основной класс для запуска в main
// *************************************************************

#ifndef DASHBOARDCORE_H
#define DASHBOARDCORE_H

//#include "DEBUG.h"
#include "CreateFiles.h"
//#include <memory>
//#include <map>
//#include <thread>

//enum class ecFilesTypes;
typedef std::map<ecFilesTypes, std::shared_ptr<FileTXT>> SP_FilesMap;

class DashboardCore 
{
public:
	DashboardCore();
	~DashboardCore();

	void StartResponeAsteriskFile(ecFilesTypes, std::string&);		   // команда на запуск потоков получения файлов
	void StopResponeAsteriskFile(ecFilesTypes, std::string&);			   // команда на остановку потоков получения файлов

	

private:
	SP_FilesMap m_filesList;


	void InitAsteriskResponseFiles(ecFilesTypes);  // создание листа с указателями на классы файлов
	
};


#endif // DASHBOARDCORE_H
