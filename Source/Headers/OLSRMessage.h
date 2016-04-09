#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <stdint.h>
#include <vector>
#include <memory>
#include "IPv6Address.h"
#include "Resources/Constants.h"
#include "Message.h"
#include "Packet.h"

class OLSRMessage
{
public:
	OLSRMessage();
    OLSRMessage(shared_ptr<Packet>);
	~OLSRMessage();
    IPv6Address mSenderHWAddr;
    IPv6Address mRecvedHWAddr; // The Interface addr receieved from (we only have 1 as of now)
    std::vector<Message> mOLSRMessages;
    uint8_t getVTime();
    // char[] serialize();

private:

    // class MessageHeader
    // {
    // public:
    //     MessageHeader(Message&);
    //     unsigned char type;
    //     unsigned char vtime;
    //     unsigned short messageSize;
    //     unsigned int originatorAddress;
    //     unsigned char timeToLive;
    //     unsigned char hopCount;
    //     unsigned short messageSequenceNumber;
    //     char[] message;

    //     char[] serialize();
    // };

};

#endif // OLSR_MESSAGE_H
