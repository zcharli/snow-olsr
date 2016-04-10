#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <iostream>
#include <memory>
#include "OLSRMessage.h"
#include "IPv6Address.h"


class Message
{
public:
     // A suggestion, MessageHeader should go into the Message class
    // since each message has a message header.
    // The MsgHeader is defined here, but used in Message.h/cpp
    // Discussion: I think MessageHeader would be more flexible as a struct
    struct MessageHeader
    {
    public:
        int copyConstructor(Message&);
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


    Message();
    ~Message();
    unsigned char getType();
    int getSize();
    uint8_t getVTime();
    virtual char* serialize();
    MessageHeader mMessageHeader;
    std::shared_ptr<IPv6Address> getOriginatorAddress();

private:
    std::shared_ptr<IPv6Address> mOriginatorAddress;
    unsigned char type;
    int size;
};

#endif // MESSAGE_H
