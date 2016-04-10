#include "Headers/OLSRMessage.h"

OLSRMessage::OLSRMessage() {}
OLSRMessage::OLSRMessage(std::shared_ptr<Packet> packet) {}
OLSRMessage::~OLSRMessage(){} 

OLSRMessage& OLSRMessage::serialize() {
    int packetLength = 4; // 4 bytes is required for packet len and seq num which are required.
    int packetSequenceNumber = 4; // temporary default value.

    std::vector<Message::MessageHeader> headers;
    for (auto& m : messages) {
        Message::MessageHeader hdr;
        hdr.copyConstructor(m);
        headers.push_back(hdr);
        packetLength += hdr.messageSize;
    }


    data.push_back(packetLength);
    data.push_back(packetSequenceNumber);

    for (int i = 0; i < headers.size(); ++i) {

    	std::vector<int> s_hdr = headers[i].serialize();
        for (int j : s_hdr) data.push_back(j);

        std::vector<int> s_msg = messages[i].serialize();
        for (int j : s_msg) data.push_back(j);
    }

    return *this;

}

int* OLSRMessage::getData() {
        return data.data();
}
