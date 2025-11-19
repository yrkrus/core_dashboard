#include "Check_ActiveLisaCalls.h"
#include "../system/Constants.h"


CheckActiveLisaCalls::CheckActiveLisaCalls()
    : m_dispether("ActiveLisaCalls",CONSTANTS::TIMEOUT::ACTIVE_CALLS_LISA)
{
}

CheckActiveLisaCalls::~CheckActiveLisaCalls()
{
}

bool CheckActiveLisaCalls::Execute()
{
    return m_lisaCalls.Execute();   
}

void CheckActiveLisaCalls::Start()
{
     m_dispether.Start([this]()
					  { return Execute(); });
}

void CheckActiveLisaCalls::Stop()
{
    m_dispether.Stop();
	printf("ActiveLisaCalls stopped!\n");
}
