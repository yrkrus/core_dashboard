//////////////////////////////////////////////////////
//													//	        
//			by Petrov Yuri 14.08.2024				//
//			  ����������� ��������   				//
//													//	
//////////////////////////////////////////////////////

#pragma once
#include <string>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <fstream>


namespace LOG {
	
	enum class Log
	{
        Log_enter                   = 0,         // ����
        Log_exit                    = 1,         // �����
        Log_auth_error              = 2,         // �� �������� �����������
        Log_exit_force              = 3,         // ����� (����� ������� force_closed)
        Log_add_queue_5000          = 4,         // ���������� � ������� 5000
        Log_add_queue_5050          = 5,         // ���������� � ������� 5050
        Log_add_queue_5000_5050     = 6,         // ���������� � ������� 5000 � 5050
        Log_del_queue_5000          = 7,         // �������� �� ������� 5000
        Log_del_queue_5050          = 8,         // �������� �� ������� 5050
        Log_del_queue_5000_5050     = 9,         // �������� �� ������� 5000 � 5050
        Log_available               = 10,        // ��������
        Log_home                    = 11,        // �����        
        Log_exodus                  = 12,        // �����
        Log_break                   = 13,        // �������
        Log_dinner                  = 14,        // ����
        Log_postvyzov               = 15,        // ���������
        Log_studies                 = 16,        // �����
        Log_IT                      = 17,        // ��
        Log_transfer                = 18,        // ��������
        Log_reserve                 = 19,        // ������
	};
	
	class Logging
	{
    public:
        Logging()	= default;
		~Logging()	= default;
    
        void createLog(Log command, int base_id);       // �������� ����
	};

    
    enum ELogType
    {
        eLogType_DEBUG  = 1,
        eLogType_INFO   = 2,
        eLogType_ERROR  = 3,
    };


    class LogToFile
    {
    public:
        LogToFile(ELogType type);
        ~LogToFile();
            
        void add(std::string message);  // ���������� ���� � ���� 
    
    private:        
        mutable std::mutex mutex;
        std::ofstream *file_log = nullptr;
        ELogType current_type;

        std::string ELogType_to_string(const ELogType &elogtype);   
    
    };
   
}




