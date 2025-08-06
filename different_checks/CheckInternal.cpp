#include "CheckInternal.h"
#include "../Constants.h"

CheckInternal::CheckInternal()
    : m_dispether("CheckInternal",CONSTANTS::TIMEOUT::CHECK_INTERNAL)
{
}

CheckInternal::~CheckInternal()
{
    Stop();
}

void CheckInternal::Start()
{
    m_dispether.Start([this]()
					  { return Execute(); });
}

void CheckInternal::Stop()
{  
    m_dispether.Stop();
	printf("CheckInternal stopped!\n");
}

bool CheckInternal::Execute()
{
   // TODO пока один return потом сделать по другому
    return m_trunkSip.Execute();   
}
  
