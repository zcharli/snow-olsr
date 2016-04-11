#include "Headers/HelloMessage.h"

HelloMessage::HelloMessage() {}
HelloMessage::~HelloMessage() {
    if(mSerializedData != NULL) {
        delete [] mSerializedData;
    }
}

uint8_t HelloMessage::getWillingness() {
    std::cout << "HelloMessage::getWillingness not yet implemented" << std::endl;
    return 0;
}

std::vector<HelloMessage::LinkMessage> HelloMessage::getLinkMessages() const {
    return mLinkMessages;
}

void HelloMessage::serialize() {
    if(mSerializedData != NULL) {
        delete [] mSerializedData;
    }
    int vCurrentIndex = 0;
    mSerializedDataSize = 26; // With header
    for (auto& linkMsg : mLinkMessages) {
        mSerializedDataSize += 4;
        mSerializedDataSize += linkMsg.neighborIfAddr.size() * 14;
    }
    mSerializedData = new char[mSerializedDataSize];

    // Make header
    // MsgType
    mSerializedData[vCurrentIndex++] = mMessageHeader.type;

    // VTime
    mSerializedData[vCurrentIndex++] = mMessageHeader.vtime;

    // MessageSize
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mSerializedDataSize - 22); // 22 is header size
    vCurrentIndex += 2;

    // Originator address
    memcpy ( mSerializedData + vCurrentIndex, mMessageHeader.originatorAddress, WLAN_HEADER_LEN );
    vCurrentIndex+=WLAN_HEADER_LEN;

    // Time to Live
    mSerializedData[vCurrentIndex++] = mMessageHeader.timeToLive;

    // Hop Count
    mSerializedData[vCurrentIndex++] = mMessageHeader.hopCount;

    // Message sequence number
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mMessageHeader.messageSequenceNumber); // 22 is header size
    vCurrentIndex += 2;

    // Make hello msg
    // Reserved section
    mSerializedData[vCurrentIndex++] = 0;
    mSerializedData[vCurrentIndex++] = 0;

    // HTime
    mSerializedData[vCurrentIndex++]  = htime;

    // Willingness
    mSerializedData[vCurrentIndex++]  = willingness;

    for(auto& msg : mLinkMessages) {
        // Link code
        *(mSerializedData + vCurrentIndex) = msg.linkCode;
        vCurrentIndex++;

        // Reserved
        *(mSerializedData + vCurrentIndex) = 0;
        vCurrentIndex++;

        // Link msg size
        uint16_t msgSize = msg.neighborIfAddr.size() * 14;
        *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(msgSize);
        vCurrentIndex+=2;

        for(auto& addr : msg.neighborIfAddr) {
            memcpy ( mSerializedData + vCurrentIndex, addr.data, WLAN_HEADER_LEN );
            vCurrentIndex += WLAN_HEADER_LEN;
        }
    }
}
