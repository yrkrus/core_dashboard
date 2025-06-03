#include <filesystem>
#include <fstream>
#include <iostream>

#include "IFile.h"
#include "InternalFunction.h"

#define ROOT_DIRECTORY_FOLDER "files/"

namespace fs = std::filesystem;
using namespace INTERNALFUNCTION;

IFile::IFile(const string &_fileName)
	: m_directory(ROOT_DIRECTORY_FOLDER)
	, m_fileName(_fileName)
	, m_counter(0)
{
}

IFile::~IFile()
{
}

//string IFile::GetFileName() const
//{
//	return m_fileName;
//}

bool IFile::CreateFile(string &_response, string &_errorDescription)
{
	// проверим есть ли папка 
	if (!fs::exists(ROOT_DIRECTORY_FOLDER) && !fs::is_directory(ROOT_DIRECTORY_FOLDER))
	{
		fs::create_directory(ROOT_DIRECTORY_FOLDER);
	}

	if (_response.empty()) 
	{
		_errorDescription = StringFormat("%s response is empty", METHOD_NAME);
		return false;
	}

	// создадим запрос
	std::ostringstream append;
	append << " > " << ROOT_DIRECTORY_FOLDER << m_fileName << "_" << m_counter; // куда будем сохранять ../files/fileName_N
	_response += append.str();	

	// отправляем запрос
	system(_response.c_str());

	if (!fs::exists(ROOT_DIRECTORY_FOLDER + m_fileName + "_" + std::to_string(m_counter)))
	{
		_errorDescription = StringFormat("%s not created file %s", METHOD_NAME, m_fileName + "_" + std::to_string(m_counter));
		return false;
	}
	
	++m_counter;
	m_list.insert(m_fileName + "_" + std::to_string(m_counter));

	return true;
}
