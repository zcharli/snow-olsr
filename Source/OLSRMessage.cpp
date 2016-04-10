#include "Headers/OLSRMessage.h"
// A implementation of OLSR agent
// http://www.cs.utexas.edu/users/acharya/Tools/ns3/ns-allinone-3.5/ns-3.5/src/routing/olsr/olsr-routing-protocol.cc

OLSRMessage::OLSRMessage() : serialized(false) {}
OLSRMessage::~OLSRMessage(){ if(serialized) delete data; }

OLSRMessage& OLSRMessage::serialize() {
    unsigned short packetLength = 4; // 4 bytes is required for packet len and seq num which are required.
    unsigned short packetSequenceNumber = 4; // temporary default value.

    std::vector<MessageHeader> headers;
    for (auto& m : messages) {
        MessageHeader hdr(m);
        headers.push_back(hdr);
        packetLength += hdr.messageSize;
    }

    if (serialized) delete data;

    data = new char[packetLength];
    data[0] = packetLength;
    data[2] = packetSequenceNumber;

    int offset = 4; // the offset in the output
    for (auto& hdr : headers) {
    	char* s_hdr = hdr.serialize();
    	for (int i = 0; i < hdr.messageSize; ++i)
    		data[offset+i] = s_hdr[i];
        offset += hdr.messageSize;
        delete s_hdr;
    }

    serialized = true;
    return *this;

}


OLSRMessage::MessageHeader::MessageHeader(Message& msg) {
    message = msg.serialize();

    type = msg.type;
    vtime = 2; // default value for now.
    messageSize = 12 + msg.size;
    // originatorAddress;
    // timeToLive;
    // hopCount;
    // messageSequenceNumber;

}


char* OLSRMessage::MessageHeader::serialize() {
    char* output = new char[messageSize];
    output[0] = type;
    output[1] = vtime;
    output[2] = messageSize;
    output[4] = originatorAddress;
    output[8] = timeToLive;
    output[9] = hopCount;
    output[10] = messageSequenceNumber;

    return output;
}
