
// ��������� �������� ��� callback  2025-04-22

#ifndef REMOTE_COMMANDS_CALLBACK_H
#define REMOTE_COMMANDS_CALLBACK_H

#include <string>

using std::string;

enum class ecCallbackStatus 
{
	failed	= 0,
	success = 1,	
};


struct StructCallback 
{
	unsigned int m_id;							// id �������
	unsigned int m_sip;							// sip ��� �����������
	unsigned int m_user_id;						// user id �� ��
	string m_phone;								// ����� �������� �� ������� ����� �������
	string m_datetime;							// ������� �����
	ecCallbackStatus m_status;					// ������ �������\�� �������
	unsigned int m_additional_field;			// ��� ������. ���� �� ������������(����� � �������)


};



#endif // REMOTE_COMMANDS_CALLBACK_H
