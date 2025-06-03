#include "CreateFiles.h"


FileTXT::FileTXT(ecFilesTypes _type)
	: IResponseFileHandler(_type)
	, m_fileName(_type)
	, m_file(nullptr)
	, m_stop(false)
{
}

FileTXT::~FileTXT()
{
	Stop();
	if (m_thread.joinable()) 
	{
		m_thread.join();
	}

	if (m_file && m_file->is_open())
	{
		m_file->close();
	}
}


// получаем файл txt
bool FileTXT::GetFile(std::string &_errorDescription)
{
	if (DEBUG_MODE) 
	{
		_errorDescription = StringFormat("Error receiving file %s, debug mode enabled", EnumToString(m_fileName));
		return false;
	}	
	
	std::unique_lock<std::shared_mutex> lock(m_mutex);		
	
	switch (m_fileName)
	{
		case ecFilesTypes::IVR:
			system(FILES::cIVRResponse.c_str());			
			break;
		case ecFilesTypes::Queue:
			break;
		case ecFilesTypes::ActiveSip:
			break;
		case ecFilesTypes::ActiveSipOperators:
			break;
		default:
			break;
	}	
	
	return true;	
}

bool FileTXT::OpenFile(std::string &_errorDescription)
{
	_errorDescription = "";
	if (!IsFileExist()) 
	{
		_errorDescription = StringFormat("File %s not exist",m_fileTXT.c_str());
		return false;
	}

	m_file = std::make_shared<std::ifstream>(EnumToString(m_fileName));
	if (!m_file->is_open()) 
	{
		_errorDescription = StringFormat("Failed to open file %s", m_fileTXT.c_str());
		return false;
	}

	return true;
}

bool FileTXT::ReadFile(std::string &_errorDescription)
{
	return true;
}


// запуск потока на доставание файла
void FileTXT::Start()
{
	m_thread = std::thread(&FileTXT::Run, this);
}

void FileTXT::Stop()
{
	m_stop = true;
}


void FileTXT::Run()
{
	std::string errorDescription;
	while (!m_stop)  // Бесконечный цикл
 	{ 
		if (!GetFile(errorDescription))
		{
			printf('/n'+errorDescription.c_str());  // TODO тут потом подумать может в лог писать!?			
		}		
		
		std::this_thread::sleep_for(std::chrono::seconds(1)); // Задержка между вызовами TODO потом ее увеличить\уменьшить!?
	}	
}
