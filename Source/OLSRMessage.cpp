#include "Headers/OLSRMessage.h"

OLSRMessage::OLSRMessage() : mSerializedData(NULL) {}
OLSRMessage::OLSRMessage(std::shared_ptr<Packet> packet)
    : mSenderHWAddr(packet->getSource()), mRecvedHWAddr(packet->getMyAddress()) {
    IPv6Address destination(packet->getDestination());
    deserializePacketBuffer(packet->getBuffer());
}
OLSRMessage::OLSRMessage(char* buffer) : mSerializedData(NULL) {

    deserializePacketBuffer(buffer);
}

OLSRMessage::~OLSRMessage() {
    if (mSerializedData != NULL) {
        delete [] mSerializedData;
    }
}

OLSRMessage& OLSRMessage::serialize() {
    if (mSerializedData != NULL) {
        delete [] mSerializedData;
    }
    mPacketLength = 4;

    for (auto* msg : messages) {
        msg->serialize();
        mPacketLength += msg->mSerializedDataSize;
        std::cout << "Serialized a ms with datasize " << msg->mSerializedDataSize << std::endl;
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
    for (auto* msg : messages) {
        memcpy ( mSerializedData + vCurrentIndex, msg->mSerializedData, msg->mSerializedDataSize);
        vCurrentIndex += msg->mSerializedDataSize;
    }

    return *this;
}

char* OLSRMessage::getData() {
    return mSerializedData;
}

void OLSRMessage::deserializePacketBuffer(char* vBuffer) {

    // Packet Length
    mPacketLength = ntohs((*(uint16_t*) vBuffer));
    vBuffer += 2;

    // Packet sequence number
    mPacketSequenceNumber = ntohs((*(uint16_t*) vBuffer));
    vBuffer += 2;

    int vBytesLeftToProccess = mPacketLength - 4;
    while (vBytesLeftToProccess > 0) {
        // Process every message
        // Get the message type
        uint8_t vMessageType = (*(uint8_t*) vBuffer);
        switch (vMessageType) {
        case M_HELLO_MESSAGE:
        {
            // Reach for the message size
            uint16_t vMessageSize = ntohs(*(uint16_t*) (vBuffer + 2));
            vMessageSize += HELLO_MSG_HEADER;
            char* vHelloMessageBuffer = new char[vMessageSize];
            memcpy ( vHelloMessageBuffer, vBuffer, vMessageSize);
            HelloMessage vHelloMessage(vHelloMessageBuffer);
            vBytesLeftToProccess -= vMessageSize;
            delete [] vHelloMessageBuffer;
            break;
        }
        case M_TC_MESSAGE:

            break;
        case M_MID_MESSAGE:
            // Not implemented
            break;
        case M_HNA_MESSAGE:
            // Not implemented
            break;
        }


    }

}
