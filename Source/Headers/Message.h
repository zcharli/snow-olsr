#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

class Message
{
public:
    Message();
    ~Message();
    unsigned char getType();
    int getSize();
    virtual char* serialize();


private:
    unsigned char type;
    int size;
};

#endif // MESSAGE_H
