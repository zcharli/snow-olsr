#include "Headers/OLSRMessage.h"
// A implementation of OLSR agent
// http://www.cs.utexas.edu/users/acharya/Tools/ns3/ns-allinone-3.5/ns-3.5/src/routing/olsr/olsr-routing-protocol.cc

OLSRMessage::OLSRMessage() {}
OLSRMessage::~OLSRMessage(){}

// char* OLSRMessage::serialize(std::vector<Message> msgs) {
//     unsigned short packetLength = 4; // 4 bytes is required for packet len and seq num which are required.
//     unsigned short packetSequenceNumber = genSeqNum();

//     std::vector<MessageHeader> headers;
//     for (auto& m : msgs) {
//         MessageHeader hdr(m);
//         headers.push_back(hdr);
//         packetLength += hdr.messageSize;
//     }

//     char output[packetLength];
//     output[0] = packetLength;
//     output[2] = packetSequenceNumber;

//     int offset = 4; // the offset in the output
//     for (auto& hdr : headers) {
//         output[offset] = hdr.serialize();
//         offset += hdr.messageSize;
//     }

//     return output;

// }


// OLSRMessage::MessageHeader::MessageHeader(Message& msg) {
//     message = msg.serialize();

//     type = msg.getType();
//     // vtime
//     messageSize = 12 + msg.size;
//     // originatorAddress;
//     // timeToLive;
//     // hopCount;
//     // messageSequenceNumber;

// }


// char* OLSRMessage::MessageHeader::serialize() {
//     char output[messageSize];
//     output[0] = type;
//     output[1] = vTime;
//     output[2] = messageSize;
//     output[4] = originatorAddress;
//     output[8] = timeToLive;
//     output[9] = hopCount;
//     output[10] = messageSequenceNumber;

//     return output;
// }
