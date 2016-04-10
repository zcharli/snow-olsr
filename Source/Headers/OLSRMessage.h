#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <memory>
#include "Resources/Constants.h"
#include "Message.h"
#include "Packet.h"
#include "IPv6Address.h"

class Message;

class OLSRMessage
{
public:
    OLSRMessage(std::shared_ptr<Packet> message);
    // Commented code below needs to be deleted. We have this stuff already in Contrants.h
    // enum MessageType {
    //     HELLO_MESSAGE = M_HELLO_MESSAGE,
    //     TC_MESSAGE    = M_TC_MESSAGE,
    // };

    OLSRMessage();
    ~OLSRMessage();

    //void setMessageType(MessageType messageType) { m_messageType = messageType; }
    // MessageType getMessageType() { return m_messageType; }
    // IPv6Address getSender() { return sender; }
    // IPv6Address getReceiver() { return receiver; }
    // Not sure why we need Hello and TC message...
    // HelloMessage& getHelloMessage() { return helloMessage; }
    // TCMessage& getTCMessage(){ return tcMessage; }
    //const IPv6Address getOriginatorAddress(){ return originatorAddress; }

    OLSRMessage& serialize();
    int* getData();

    std::vector<Message> messages;
    IPv6Address mSenderHWAddr; // The last hop sender addr from, may NOT be the originating addr
    IPv6Address mRecvedHWAddr; // The Interface addr receieved from (we only have 1 as of now)
    //uint8_t getVTime();

private:
    //MessageType m_messageType;
    // IPv6Address sender;
    // IPv6Address receiver;
    //const IPv6Address originatorAddress;

    //TCMessage tcMessage;
    //HelloMessage helloMessage;

    std::vector<int> data;

    // Unique OLSRPacket Attributes
    uint16_t mPacketLength;
    uint16_t mPacketSequenceNumber;

};

#endif // OLSR_MESSAGE_H
