#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "RemoteCommands.h"
#include "Queue.h"
#include "Log.h"
#include "CallInfo.h"
#include "different_checks/Check_TrunkSip.h"

template<typename T>
T StringToEnum(const std::string&);

template<typename T>
std::string EnumToString(T);

template<>
ecQueueNumber StringToEnum<ecQueueNumber>(const std::string&);

template<>
std::string EnumToString<ecQueueNumber>(ecQueueNumber);

template<>
std::string EnumToString<ecCommand>(ecCommand);

template<>
std::string EnumToString<ecCommandType>(ecCommandType);

template<>
std::string EnumToString<ecLogType>(ecLogType);

template<>
ecTrunkState StringToEnum<ecTrunkState>(const std::string&);

template<>
std::string EnumToString<ecTrunkState>(ecTrunkState);

template<>
std::string EnumToString<ecCallInfoTable>(ecCallInfoTable);

#endif // UTILS_H 
