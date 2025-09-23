#ifndef UTILS_H
#define UTILS_H

/*

    @file custom_cast.h 

*/

#include <string>
#include "RemoteCommands.h"
#include "Queue.h"
#include "Log.h"
#include "CallInfo.h"
#include "SmsInfo.h"
#include "IVR.h"
#include "different_checks/Check_TrunkSip.h"
#include "IAsteriskApplication.h"

namespace custom_cast 
{
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

    template<>
    std::string EnumToString<ecSmsInfoTable>(ecSmsInfoTable);

    template<>
    ecSmsCode StringToEnum<ecSmsCode>(const std::string&);

    template<>
    ecAsteriskApp StringToEnum<ecAsteriskApp>(const std::string&);

    template<>
    ecCallerId StringToEnum<ecCallerId>(const std::string&);

    template<>
    std::string EnumToString<ecCallerId>(ecCallerId);

    template<>
    ecAsteriskState StringToEnum<ecAsteriskState>(const std::string&);

    template<>
    std::string EnumToString<ecAsteriskState>(ecAsteriskState);

}




#endif // UTILS_H 
