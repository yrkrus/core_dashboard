#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "RemoteCommands.h"
#include "Queue.h"
#include "Log.h"

template<typename T>
T StringToEnum(const std::string&);

template<typename T>
std::string EnumToString(T);

template<>
EQueueNumber StringToEnum<EQueueNumber>(const std::string&);

template<>
std::string EnumToString<EQueueNumber>(EQueueNumber);

template<>
std::string EnumToString<ECommand>(ECommand);

template<>
std::string EnumToString<ECommandType>(ECommandType);

template<>
std::string EnumToString<ELogType>(ELogType);


#endif // UTILS_H 
