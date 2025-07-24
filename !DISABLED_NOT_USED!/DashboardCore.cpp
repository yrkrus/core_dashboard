#include "DashboardCore.h"

DashboardCore::DashboardCore()
{
	// TODO потом сдеалить через список
	InitAsteriskResponseFiles(ecFilesTypes::IVR);
}

DashboardCore::~DashboardCore()
{
}

void DashboardCore::StartResponeAsteriskFile(ecFilesTypes _typefile, std::string &_errorDescriptions)
{
	_errorDescriptions = "";

	auto it = m_filesList.find(_typefile);
	if (it == m_filesList.end()) 
	{
		//_errorDescriptions = StringFormat(EnumToString(_typefile) );
		return;
	}

	it->second->Start();
}

void DashboardCore::StopResponeAsteriskFile(ecFilesTypes _typefile, std::string &_errorDescriptions)
{
	_errorDescriptions = "";

	auto it = m_filesList.find(_typefile);
	if (it == m_filesList.end())
	{
		return;
	}

	it->second->Stop();
}

void DashboardCore::InitAsteriskResponseFiles(ecFilesTypes _typefile)
{
	auto file = std::make_shared<FileTXT>(_typefile);

	m_filesList[_typefile] = file;
	
}


