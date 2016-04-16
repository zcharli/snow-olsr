#include "Headers/Message.h"

Message::Message() : mSerializedData(NULL), mOriginatorAddress(NULL) {
}

Message::~Message() {
    //std::cout << "Message destructor" << std::endl;
    if (mSerializedData != NULL) {
        std::cout << "del [] mSerializedData" << std::endl;
        delete [] mSerializedData;
        mSerializedData = NULL;
    }
}

std::shared_ptr<MACAddress> Message::getOriginatorAddress() {
    if (mOriginatorAddress != NULL) {
        return mOriginatorAddress;
    }
    std::cout << "Implementation for getOriginatorAddress missing" << std::endl;
    return NULL;
}

unsigned char Message::getType() {
    return type;
}

int Message::getSize() {
    return size;
}

uint8_t Message::getVTime() {
    // temporary function, in fact we need away to extract vtime from msg header
    std::cout << "Message::getVTime is not yet implemented" << std::endl;
    return 0;
}

int Message::MessageHeader::copyConstructor(Message& msg) {
    // message = msg.serialize();

    // type = msg.getType();
    // vtime = 2; // default value for now.
    // messageSize = 20 + msg.getSize();
    //   BTW, originatorAddress is 14 bytes, WLAN_ADDR_LEN

    // originatorAddress;
    // timeToLive;
    // hopCount;
    // messageSequenceNumber;
    return 0;
}

void Message::serialize() {
    PRINTLN(Dam im the base class)
}
void Message::deserialize(char* buffer) {
        PRINTLN(Dam im the base class)
}
