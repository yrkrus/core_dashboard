#include "IResponseFileHandler.h"
#include "CreateFiles.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

template<>
std::string EnumToString(ecFilesTypes _type)
{
	static Files_map filesMap =
	{
		{ecFilesTypes::IVR,					FILES::cIVRName},
		{ecFilesTypes::Queue,				"Queue"},
		{ecFilesTypes::ActiveSip,			"ecActiveSip"},
		{ecFilesTypes::ActiveSipOperators,	"ecActiveSipOperators"},
	};

	auto it = filesMap.find(_type);
	if (it == filesMap.end())
	{
		return "Unknown";
	}

	return it->second;
}

IResponseFileHandler::IResponseFileHandler(ecFilesTypes _type)
	: m_fileTXT(FILES::cFOLDER + EnumToString(_type))
{	
	if (!fs::exists(FILES::cFOLDER) && !fs::is_directory(FILES::cFOLDER)) 
	{
		fs::create_directory(FILES::cFOLDER);
	}
}

IResponseFileHandler::~IResponseFileHandler()
{}

bool IResponseFileHandler::IsFileExist() const
{
	return fs::exists(m_fileTXT);
}
