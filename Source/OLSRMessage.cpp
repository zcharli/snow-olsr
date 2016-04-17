#include "Headers/OLSRMessage.h"

OLSRMessage::OLSRMessage() : mSerializedData(nullptr) {}
OLSRMessage::OLSRMessage(std::shared_ptr<Packet> packet)
    : mSenderHWAddr(packet->getSource()), mRecvedHWAddr(packet->getMyAddress()), mSerializedData(nullptr) {
    // Initialize everything to null or fill up MAC addreses used.
    MACAddress destination(packet->getDestination());
    deserializePacketBuffer(packet->getBuffer() + WLAN_HEADER_LEN);
}
OLSRMessage::OLSRMessage(char* buffer) : mSerializedData(nullptr) {

    deserializePacketBuffer(buffer);
}

OLSRMessage::~OLSRMessage() {
    //std::cout << "OLSRMessage destructor" << std::endl;

    if (mSerializedData != nullptr) {
        std::cout << "del [] OLSRMessage" << std::endl;
        delete [] mSerializedData;
        std::cout << "deleted OLSRMessage Successfully" << std::endl;
        mSerializedData = nullptr;
    }
}

OLSRMessage& OLSRMessage::serialize() {
    //std::cout << "Seralizing hello message" << std::endl;
    if (mSerializedData != nullptr) {
        std::cout << "OLSR serialize was called for the second time" << std::endl;
        delete [] mSerializedData;
        mPacketLength = 0;
        mSerializedData = nullptr;
    }
    mPacketLength = 4; // mPacketLength and mSequenceNumber
    for (std::shared_ptr<Message>& msg : messages) {
        msg->serialize();
        mPacketLength += msg->mSerializedDataSize;
        // std::cout << "Serialized a ms with datasize " << msg->mSerializedDataSize << std::endl;
    }
    std::cout << "mSerializedData = new char[" <<mPacketLength <<"]; size " << mPacketLength << std::endl;
    mSerializedData = new char[mPacketLength];
    int vCurrentIndex = 0;

    // Packet lenght
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mPacketLength);
    vCurrentIndex += 2;

    // Packet Sequence number
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mPacketSequenceNumber);
    vCurrentIndex += 2;

    // The messages
    for (std::shared_ptr<Message>& msg : messages) {
        std::cout << "Serializing OLSR seralize" <<std::endl;
        memcpy ( mSerializedData + vCurrentIndex, msg->mSerializedData, msg->mSerializedDataSize);
        vCurrentIndex += msg->mSerializedDataSize;
    }
    std::cout << "Finished OLSR seralize" <<std::endl;
    return *this;
}

int OLSRMessage::getPacketSize() {
    return mPacketLength;
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
            std::cout << "char* vHelloMessageBuffer = new char["<< vMessageSize<<"];" << std::endl;
            char* vHelloMessageBuffer = new char[vMessageSize];
            memcpy ( vHelloMessageBuffer, vBuffer, vMessageSize);
    std::cout << "make_shared OLSRMessage hello message" << std::endl;

            std::shared_ptr<HelloMessage> vHelloMessage = std::make_shared<HelloMessage>(vHelloMessageBuffer);
            messages.push_back(vHelloMessage);
            vBytesLeftToProccess -= vMessageSize;
            mOriginatorAddress = vHelloMessage->getOriginatorAddress();
            std::cout << "delete [] vHelloMessageBuffer" << std::endl;
            delete [] vHelloMessageBuffer;
            break;
        }
        case M_TC_MESSAGE:
        {
            uint16_t vMessageSizeTC = ntohs(*(uint16_t*) (vBuffer + 2));
            vMessageSizeTC += HELLO_MSG_HEADER;
            std::cout << "char* vTCMessageBuffer = new char["<< vMessageSizeTC<<"];" << std::endl;
            char* vTCMessageBuffer = new char[vMessageSizeTC];
            memcpy ( vTCMessageBuffer, vBuffer, vMessageSizeTC);
    std::cout << "make_shared OLSRMessage TC message" << std::endl;

            std::shared_ptr<TCMessage> vTCMessage = std::make_shared<TCMessage>(vTCMessageBuffer);
            messages.push_back(vTCMessage);
            vBytesLeftToProccess -= vMessageSizeTC;
            mOriginatorAddress = vTCMessage->getOriginatorAddress();
            std::cout << "delete [] vTCMessageBuffer" << std::endl;
            delete [] vTCMessageBuffer;
            break;
        }
        case M_MID_MESSAGE:
            // Not implemented
            break;
        case M_HNA_MESSAGE:
            // Not implemented
            break;
        }
    }
}
