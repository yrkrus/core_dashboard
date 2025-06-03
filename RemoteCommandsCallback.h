
// удаленные комманды дл€ callback  2025-04-22

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
	unsigned int m_id;							// id запроса
	unsigned int m_sip;							// sip кто инициировал
	unsigned int m_user_id;						// user id по Ѕƒ
	string m_phone;								// номер телефона на который будем звонить
	string m_datetime;							// текущее врем€
	ecCallbackStatus m_status;					// статус успешно\не успешно
	unsigned int m_additional_field;			// доп статус. пока не используетс€(может в будещем)


};



#endif // REMOTE_COMMANDS_CALLBACK_H
