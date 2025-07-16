// ����� ��� ���������� ������������ ������

#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include "Command.h"
#include "Event.h"

#ifndef DISPATHER_H 
#define DISPATHER_H

class Dispather 
{
private:
	std::string						m_name;			// ��� ����������
	std::queue<SP_Command>			m_commandQueue; // ������ � ���������
	std::queue<SP_Event>			m_eventQueue;	// ������ � ������������ ���������

	bool							m_isRunnnig;	

	std::thread						m_thread;		
	mutable std::recursive_mutex	m_mutex;
	Event							m_queueIsNotEmptyEvent;

	bool PopCommand(SP_Command &_command, SP_Event &_event);

	void DispatheringThread();
	void ClearCommandQueue();

public:
	Dispather(const std::string _name);
	~Dispather();

	void Start();
	void Stop();

	bool IsRunning() const;

	SP_Event PostCommand(SP_Command command);

};




#endif //DISPATHER_H