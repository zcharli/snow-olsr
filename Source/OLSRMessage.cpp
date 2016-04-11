#include "Headers/OLSRMessage.h"

OLSRMessage::OLSRMessage() {}
OLSRMessage::OLSRMessage(std::shared_ptr<Packet> packet) {}
OLSRMessage::~OLSRMessage() {}

OLSRMessage& OLSRMessage::serialize() {
    if (mSerializedData != NULL) {
        delete [] mSerializedData;
    }
    mPacketLength = 4;

    for (auto& msg : messages) {
        msg.serialize();
        mPacketLength += msg.mSerializedDataSize;
    }

    mSerializedData = new char[mPacketLength];
    int vCurrentIndex = 0;

    // Packet lenght
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mPacketLength);
    vCurrentIndex += 2;

    // Packet Sequence number
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mPacketSequenceNumber);
    vCurrentIndex += 2;

    // The messages
    for (auto& msg : messages) {
        memcpy ( mSerializedData + vCurrentIndex, msg.mSerializedData, msg.mSerializedDataSize);
        vCurrentIndex += msg.mSerializedDataSize;
    }

    return *this;
}

char* OLSRMessage::getData() {
    return mSerializedData;
}
