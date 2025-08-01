
#include "Dispather.h"

bool Dispather::PopCommand(SP_Command &_command, SP_Event &_event)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if (m_commandQueue.empty()) 
	{
		return false;
	}

	_command = m_commandQueue.front();
	_event = m_eventQueue.front();

	m_commandQueue.pop();
	m_eventQueue.pop();

	return true;
}

void Dispather::DispatheringThread()
{
	m_isRunnnig = true;

	while (m_isRunnnig) 
	{
		SP_Command command;
		SP_Event event;
		
		// достаем новую команду
		while (PopCommand(command, event)) 
		{
			command->Execute();
			event->Set();
		}

		m_queueIsNotEmptyEvent.Wait();
	}
}

void Dispather::ClearCommandQueue()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::queue<SP_Command> emptyCommand;
	std::queue<SP_Event> emptyEvent;

	std::swap(m_commandQueue, emptyCommand);
	std::swap(m_eventQueue, emptyEvent);
}

Dispather::Dispather(const std::string _name)
	: m_name(_name)
	, m_isRunnnig(false)
{
}

Dispather::~Dispather()
{
	Stop();
}

bool Dispather::IsRunning() const
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return m_isRunnnig;
}

SP_Event Dispather::PostCommand(SP_Command command)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_commandQueue.push(command);

	SP_Event event(new Event("CommandCompletionEvent"));
	m_eventQueue.push(event);

	m_queueIsNotEmptyEvent.Set();

	return event;
}

void Dispather::Stop() 
{
	if (!m_isRunnnig) 
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_isRunnnig = false;

	// очистка команд
	ClearCommandQueue();

	m_queueIsNotEmptyEvent.Set();

	if (m_thread.joinable()) 
	{
		m_thread.join();
	}	

	printf("Dispather %s stopped!", m_name.c_str());
}

void Dispather::Start()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (m_isRunnnig) 
	{
		return;
	}

	m_thread = std::thread(&Dispather::DispatheringThread, this);

	printf("Dispather %s starded!", m_name.c_str());
}
