#include <memory>

#ifndef COMMAND_H 
#define COMMAND_H

class Command 
{
public:
	virtual ~Command() {};
	virtual bool Execute() = 0;
};

using SP_Command = std::shared_ptr<Command>;

#endif //COMMAND_H