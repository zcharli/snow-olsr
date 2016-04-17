#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <iostream>
#include <memory>
#include "MACAddress.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"
#include <netinet/in.h>


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
        uint8_t type;
        uint8_t vtime;
        uint16_t messageSize;
        char originatorAddress[WLAN_ADDR_LEN];
        uint8_t timeToLive;
        uint8_t hopCount;
        uint16_t messageSequenceNumber;
        //std::vector<int> message;
        void serialize();
    };

    Message();
    virtual ~Message();
    uint8_t getType();
    int getSize();
    uint8_t getVTime();
    virtual void serialize();
    virtual void deserialize(char*);
    MessageHeader mMessageHeader;
    std::shared_ptr<MACAddress> getOriginatorAddress();
    int mSerializedDataSize;
    char* mSerializedData;
    std::shared_ptr<MACAddress> mOriginatorAddress;
    unsigned char type;
    int size;
};

#endif // MESSAGE_H
