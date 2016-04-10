#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <memory>
#include "Resources/Constants.h"
#include "Message.h"
#include "Packet.h"
#include "TCMessage.h"
#include "HelloMessage.h"
#include "IPv6Address.h"

class OLSRMessage
{
public:
    OLSRMessage(std::shared_ptr<Packet> message);
    enum MessageType {
        HELLO_MESSAGE = M_HELLO_MESSAGE,
        TC_MESSAGE    = M_TC_MESSAGE,
    };

    OLSRMessage();
    ~OLSRMessage();

    void setMessageType(MessageType messageType) { m_messageType = messageType; }
    MessageType getMessageType() { return m_messageType; }
    IPv6Address getSender() { return sender; }
    IPv6Address getReceiver() { return receiver; }
    HelloMessage& getHelloMessage() { return helloMessage; }
    TCMessage& getTCMessage(){ return tcMessage; }
    const IPv6Address getOriginatorAddress(){ return originatorAddress; }

    OLSRMessage& serialize();
    char* getData();

    std::vector<Message> messages;
    IPv6Address mSenderHWAddr;
    IPv6Address mRecvedHWAddr; // The Interface addr receieved from (we only have 1 as of now)
    uint8_t getVTime();

private:
    MessageType m_messageType;
    IPv6Address sender;
    IPv6Address receiver;
    const IPv6Address originatorAddress;

    TCMessage tcMessage;
    HelloMessage helloMessage;

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
