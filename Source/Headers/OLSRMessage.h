#ifndef OLSR_MESSAGE_H
#define OLSR_MESSAGE_H

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <netinet/in.h>
#include "Resources/Constants.h"
#include "Message.h"
#include "Packet.h"
#include "MACAddress.h"
#include "HelloMessage.h"
#include "TCMessage.h"

class Message;

class OLSRMessage
{
public:
    OLSRMessage(std::shared_ptr<Packet> message);
    OLSRMessage();
    OLSRMessage(char*);
    ~OLSRMessage();

    OLSRMessage& serialize();
    OLSRMessage& deserialize();

    char* getData();
    int getPacketSize();

    std::vector<std::shared_ptr<Message>> messages;
    MACAddress mSenderHWAddr; // The last hop sender addr from, may NOT be the originating addr
    MACAddress mRecvedHWAddr; // The Interface addr receieved from (we only have 1 as of now)
    std::shared_ptr<MACAddress> mOriginatorAddress;
    uint16_t mPacketLength;
    uint16_t mPacketSequenceNumber;

private:
    void deserializePacketBuffer(char*);
    char mSerializedData[MAX_BUF];
    // Unique OLSRPacket Attributes

};

#endif // OLSR_MESSAGE_H
