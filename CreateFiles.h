// *************************************************************
//						 ПУТИ К ФАЙЛАМ
//			  + абстрактный класс получения файла
// *************************************************************

#ifndef CREATEFILES_H
#define CREATEFILES_H

#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include "DEBUG.h"
#include "IResponseFileHandler.h"
#include "InternalFunction.h"

using utils::StringFormat;


namespace FILES
{
	static std::string cFOLDER = "files/";		// папка с файлами

	// для IVR
	static std::string cIVRName			= "IVR.txt";
	static std::string cIVRCommands		= "Playback|lukoil|ivr-3";
	static std::string cIVRCommandsEx1	= "IVREXT";
	static std::string cIVRCommandsEx2	= "Spasibo";
	static std::string cIVRResponse		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + cIVRCommands + "\" " + " | grep -v \"" + cIVRCommandsEx1 + "\" " + " | grep -v \"" + cIVRCommandsEx2 + "\" > " + cFOLDER + cIVRName;
}


enum class ecFilesTypes
{
	IVR,
	Queue,
	ActiveSip,
	ActiveSipOperators
};

typedef std::map<ecFilesTypes, std::string> Files_map;
typedef std::shared_ptr<std::ifstream> SP_File;


class FileTXT : public IResponseFileHandler
{	
public:
	FileTXT(ecFilesTypes);
	~FileTXT();
		

	void Start();					// запуск потока на доставание файла
	void Stop();					// остановка потока 
	
	bool OpenFile(std::string &_errorDescription) override;
	bool ReadFile(std::string &_errorDescription) override;

	
private:
	ecFilesTypes		m_fileName;
	SP_File				m_file;			// сам файл который будет смотреться
	std::atomic<bool>	m_stop; 
	std::thread			m_thread;

	bool GetFile(std::string &_errorDescription) override;
	void Run();							// запуск потока на получение файла
};


#endif //CREATEFILES_H