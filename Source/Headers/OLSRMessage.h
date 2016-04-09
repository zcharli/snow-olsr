#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <vector>

#include "Message.h"

class OLSRMessage
{
public:
	OLSRMessage();
	~OLSRMessage();
    
    char[] serialize();

private:
    
    class MessageHeader
    {
    public:
        MessageHeader(Message&);
        unsigned char type;
        unsigned char vtime;
        unsigned short messageSize;
        unsigned int originatorAddress;
        unsigned char timeToLive;
        unsigned char hopCount;
        unsigned short messageSequenceNumber;
        char[] message;
        
        char[] serialize();
    }
	
};

#endif // OLSR_MESSAGE_H