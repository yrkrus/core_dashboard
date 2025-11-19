#ifndef CHECK_ACTIVE_LISA_CALLS_H
#define CHECK_ACTIVE_LISA_CALLS_H

#include <memory>
#include "../interfaces/IPotokDispether.h"
#include "../core/ActiveLisa.h"


class CheckActiveLisaCalls
{
private:
    ActiveLisa      m_lisaCalls; 
    IPotokDispether m_dispether;   

    bool Execute();

public:
    CheckActiveLisaCalls();
    ~CheckActiveLisaCalls(); 
    
    void Start();  
    void Stop();  
};
using SP_CheckActiveLisaCalls = std::shared_ptr<CheckActiveLisaCalls>; 


#endif //CHECK_ACTIVE_LISA_CALLS_H