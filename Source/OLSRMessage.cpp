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
    for (int i = 0; i < headers.size(); ++i) {//(auto& hdr : headers) {

    	char* s_hdr = headers[i].serialize();
    	for (int j = 0; j < headers[i].messageSize; ++j)
    		data[offset+j] = s_hdr[j];
        offset += headers[i].messageSize;
        delete s_hdr;

        char* s_msg = messages[i].serialize();
        for (int j = 0; j < messages[i].getSize(); ++j)
        	data[offset+j] = s_msg[j];
        offset += messages[i].getSize();
        delete s_msg;
    }

    serialized = true;
    return *this;

}
