// *************************************************************
//						 Режим DEBUG
//			  + абстрактный класс получения файла
// *************************************************************

#ifndef DEBUG_H
#define DEBUG_H

#if defined(_MSC_VER)
#define METHOD_NAME() std::string(__FUNCSIG__)
#else
#define METHOD_NAME() std::string(__PRETTY_FUNCTION__)
#endif

#include <string>

static const bool DEBUG_MODE = false;



#endif // DEBUG_H
