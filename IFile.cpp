
#include <fstream>
#include <iostream>
#include <shared_mutex>

#include "IFile.h"
#include "InternalFunction.h"
#include "DEBUG.h"


using namespace utils;


IFile::IFile()
{
}

IFile::~IFile()
{
}

bool IFile::GetRequest(const char *_request, string &_response, string &_errorDescription)
{
	_errorDescription = "";
	
	// поток для хранения
	std::ostringstream result;

	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(_request, "r"), pclose);
	if (!pipe) 
	{
		_errorDescription = StringFormat("%s popen() failed\n", METHOD_NAME);
		return false;
	}

	// читаем из буфера по 128 символов
	char buffer[128];
	while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) 
	{
		result << buffer;
	}
	
	_response = result.str();

	if (_response.empty()) 
	{
		_errorDescription = StringFormat("%s response is empty", METHOD_NAME);
		return false;
	}

	return true;
}


bool IFile::CreateData(string &_request, string &_errorDescription)
{
	if (_request.empty())
	{
		_errorDescription = StringFormat("%s request is empty\n", METHOD_NAME);
		return false;
	}
	
	std::string response;
	if (!GetRequest(_request.c_str(), response, _errorDescription))
	{
		return false;
	}	

	// TODO эту проверку в общем не нужно делаьб, но на всякий случай сделаем, мало ли
	// т.к. проверка на пустое идет в GetRequest() 
	if (!response.empty()) 
	{
		std::unique_lock<std::shared_mutex> write_lock(m_mutex); // lock при записи 
		m_listRaw.push_front(response);	// вставим в начало очереди
	}

	return true;
}

bool IFile::IsExistRaw()
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return !m_listRaw.empty();
}

std::deque<string> IFile::GetRawAll() const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return m_listRaw;
}

std::string IFile::GetRawLast() const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);

	if (m_listRaw.empty()) 
	{
		return "";		
	}

	return m_listRaw.back();
}

std::string IFile::GetRawFirst() const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);

	if (m_listRaw.empty())
	{
		return "";
	}

	return m_listRaw.front();
}

void IFile::DeleteRawLast()
{
	std::unique_lock<std::shared_mutex> write_lock(m_mutex); // lock при удалении 
	
	if (!m_listRaw.empty())
	{
		m_listRaw.pop_back();
	}
}

void IFile::DeleteRawAll()
{
	std::unique_lock<std::shared_mutex> write_lock(m_mutex); // lock при удалении
	m_listRaw.clear();
}
	
