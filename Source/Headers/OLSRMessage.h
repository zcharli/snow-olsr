#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <vector>
#include <algorithm>

#include "Message.h"

class OLSRMessage
{
public:
	OLSRMessage();
	~OLSRMessage();

    OLSRMessage& serialize();
    char* getData();

    std::vector<Message> messages;

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
        char* message;

        char* serialize();
    };

    char* data;
    bool serialized;

};

#endif // OLSR_MESSAGE_H
