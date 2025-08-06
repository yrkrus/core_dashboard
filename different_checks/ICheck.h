#ifndef ICHECK_H
#define ICHECK_H

class ICheck
{
public:
    ICheck();
    virtual ~ICheck();

    virtual bool Execute() = 0;
};

#endif //ICHECK_H