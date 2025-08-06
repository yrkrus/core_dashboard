// *************************************************************
//						АВТОРИЗАЦИЯ
// *************************************************************
#ifndef AUTH_H
#define AUTH_H

#include <string>

namespace AUTH 
{
    namespace MYSQL 
    {
         static const std::string HOST		= "10.34.222.19";
        //static const std::string BD		= "dashboard";
        static const std::string BD		= "dashboard_test";
        static const std::string LOGIN		= "zabbix";
        static const std::string PASSWORD	= "UFGq4kZMNIMwxTzV";
    }
   
    namespace FIREBIRD 
    {

    }   
}




#endif // AUTH_H