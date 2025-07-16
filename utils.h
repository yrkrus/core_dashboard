#include <string>
#include "RemoteCommands.h"
#include "Queue.h"

#ifndef UTILS_H
#define UTILS_H

using std::string;
using remote::ECommand;
using remote::ECommandType;

template<typename T>
T StringToEnum(const std::string&);

template<typename T>
std::string EnumToString(T);

template<>
EQueueNumber StringToEnum<EQueueNumber>(const std::string&);

template<>
string EnumToString<EQueueNumber>(EQueueNumber);

template<>
string EnumToString<ECommand>(ECommand);

template<>
string EnumToString<ECommandType>(ECommandType);



#endif // 
