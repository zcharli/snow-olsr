#include "Headers/OLSRMessage.h"

OLSRMessage::OLSRMessage() : serialized(false) {}
OLSRMessage::OLSRMessage(std::shared_ptr<Packet> packet) : serialized(false) {
    // deserialize!
}
OLSRMessage::~OLSRMessage(){ if(serialized) delete data; }

OLSRMessage& OLSRMessage::serialize() {
    unsigned short packetLength = 4; // 4 bytes is required for packet len and seq num which are required.
    unsigned short packetSequenceNumber = 4; // temporary default value.

    std::vector<Message::MessageHeader> headers;
    for (auto& m : messages) {
        Message::MessageHeader hdr;
        hdr.copyConstructor(m);
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
