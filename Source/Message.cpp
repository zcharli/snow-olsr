#include "Headers/Message.h"

Message::Message() {}

Message::~Message() {}

std::shared_ptr<IPv6Address> Message::getOriginatorAddress() {
    if(mOriginatorAddress != NULL) {
        return mOriginatorAddress;
    }
    std::cout << "Implementation for getOriginatorAddress missing" << std::endl;
    return NULL;
}

char* Message::serialize() {
    return NULL;
}

unsigned char Message::getType() {
    return type;
}

int Message::getSize(){
    return size;
}

uint8_t Message::getVTime() {
    // temporary function, in fact we need away to extract vtime from msg header
    std::cout << "Message::getVTime is not yet implemented" << std::endl;
    return 0;
}

int Message::MessageHeader::copyConstructor(Message& msg) {
    message = msg.serialize();

    type = msg.getType();
    vtime = 2; // default value for now.
    messageSize = 20 + msg.getSize();
    /*  BTW, originatorAddress is 14 bytes, WLAN_ADDR_LEN */

    // originatorAddress;
    // timeToLive;
    // hopCount;
    // messageSequenceNumber;
    return 0;
}

char* Message::MessageHeader::serialize() {
    char* output = new char[messageSize];
    output[0] = type;
    output[1] = vtime;
    output[2] = messageSize;
    output[4] = originatorAddress;
    output[18] = timeToLive;
    output[19] = hopCount;
    output[20] = messageSequenceNumber;

    return output;
}
