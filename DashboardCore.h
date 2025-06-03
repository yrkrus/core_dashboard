// *************************************************************
//			 �������� ����� ��� ������� � main
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

	void StartResponeAsteriskFile(ecFilesTypes, std::string&);		   // ������� �� ������ ������� ��������� ������
	void StopResponeAsteriskFile(ecFilesTypes, std::string&);			   // ������� �� ��������� ������� ��������� ������

	

private:
	SP_FilesMap m_filesList;


	void InitAsteriskResponseFiles(ecFilesTypes);  // �������� ����� � ����������� �� ������ ������
	
};


#endif // DASHBOARDCORE_H
