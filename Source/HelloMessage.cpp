#include "Headers/HelloMessage.h"

HelloMessage::HelloMessage()  {
    mMessageHeader.type = M_HELLO_MESSAGE;
}
HelloMessage::~HelloMessage() {
    //std::cout << "HelloMessage destructor" << std::endl;
}

HelloMessage::HelloMessage(char* buffer) {
    mMessageHeader.type = M_HELLO_MESSAGE;
    deserialize(buffer);
}

uint8_t HelloMessage::getWillingness() {
    // std::cout << "HelloMessage::getWillingness not yet implemented" << std::endl;
    return 0;
}

std::vector<HelloMessage::LinkMessage> HelloMessage::getLinkMessages() const {
    return mLinkMessages;
}

void HelloMessage::deserialize(char* buffer) {
    // MsgType
    mMessageHeader.type = (*(uint8_t*) buffer);
    buffer++;
    type = mMessageHeader.type;

    // VTime
    mMessageHeader.vtime = (*(uint8_t*) buffer);
    buffer++;

    // MessageSize
    mMessageHeader.messageSize = ntohs((*(uint16_t*) buffer));
    buffer += 2;
    int vTotalMsgSize = mMessageHeader.messageSize - 4;
    size = vTotalMsgSize;

    // Originator address
    memcpy(mMessageHeader.originatorAddress, buffer, WLAN_ADDR_LEN);
    buffer += WLAN_ADDR_LEN;
    // std::cout << "make_shared MACAddress hello message" << std::endl;
    mOriginatorAddress = std::make_shared<MACAddress>(mMessageHeader.originatorAddress);

    // Time to Live
    mMessageHeader.timeToLive = (*(uint8_t*) buffer);
    buffer++;

    // Hop Count
    mMessageHeader.hopCount = (*(uint8_t*) buffer);
    buffer++;

    // Message sequence number
    mMessageHeader.messageSequenceNumber = ntohs((*(uint16_t*) buffer));
    buffer += 2;

    // Deserialize the actual message now lol
    // std::cout << "After deserializing the header, we still have " << mMessageHeader.messageSize << " bytes left" << std::endl;

    // Skip the reserved
    buffer += 2;

    // Htime
    htime = (*(uint8_t*) buffer);
    buffer++;

    // Willingness
    willingness = (*(uint8_t*) buffer);
    buffer++;
    while (vTotalMsgSize > 0) {
        // Link code
        uint8_t vAdvertisedNeightborLinkCode = (*(uint8_t*) buffer);
        buffer++;
        vTotalMsgSize--;

        // Skip reserved
        buffer++;
        vTotalMsgSize--;

        // Link message size
        uint16_t vLinkMessageSize = ntohs((*(uint16_t*) buffer));
        buffer += 2;
        vTotalMsgSize -= 2;

        // Create advertised neightbours from links
        int vNumLinks = vLinkMessageSize / WLAN_ADDR_LEN;
        LinkMessage vLink;
        vLink.linkCode = vAdvertisedNeightborLinkCode;
        while (vNumLinks--) {
            char vAdvertisedNeighborInterfaceAddrBuffer[WLAN_ADDR_LEN];
            memcpy(vAdvertisedNeighborInterfaceAddrBuffer, buffer, WLAN_ADDR_LEN);
            MACAddress vAdvertisedNeighborInterfaceAddr(vAdvertisedNeighborInterfaceAddrBuffer);
            vLink.neighborIfAddr.push_back(vAdvertisedNeighborInterfaceAddr);
            buffer += WLAN_ADDR_LEN;
            vTotalMsgSize -= WLAN_ADDR_LEN;
            // std::cout << "Hello msg deserialization " << std::endl;
        }
        mLinkMessages.push_back(vLink);
    }

}

void HelloMessage::serialize() {
    //std::cout << "Seralizing hello message" << std::endl;
    // if (mSerializedData != nullptr) {
    //     std::cout << "Found a non nullptr serialize data, deleting it" << std::endl;
    //     delete [] mSerializedData;
    //     mSerializedDataSize = 0;
    //     mSerializedData = nullptr;
    // }
    int vCurrentIndex = 0;
    mSerializedDataSize = HELLO_MSG_HEADER + 4; // With header
    for (auto& linkMsg : mLinkMessages) {
        mSerializedDataSize += 4;
        mSerializedDataSize += linkMsg.neighborIfAddr.size() * WLAN_ADDR_LEN;
    }
    // std::cout << "Hello message serialization news" << std::endl;
    //mSerializedData = new char[mSerializedDataSize];

    // Make header
    // MsgType
    mSerializedData[vCurrentIndex++] = mMessageHeader.type;

    // VTime
    mSerializedData[vCurrentIndex++] = mMessageHeader.vtime;

    // MessageSize
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mSerializedDataSize - HELLO_MSG_HEADER); // 22 is header size
    // Test
    //uint16_t test = ntohs((*(uint16_t*) (mSerializedData + vCurrentIndex)));
    // std::cout << test << std::endl;
    vCurrentIndex += 2;

    // Originator address
    memcpy ( mSerializedData + vCurrentIndex, mMessageHeader.originatorAddress, WLAN_ADDR_LEN );
    vCurrentIndex += WLAN_ADDR_LEN;

    // Time to Live (Decremented!)
    // if (mMessageHeader.timeToLive - 1 == 0) {
    //     PRINTLN(Time to live for this Hello message is zero so requires attention)
    // }
    mSerializedData[vCurrentIndex++] = mMessageHeader.timeToLive - 1;

    // Hop Count
    mSerializedData[vCurrentIndex++] = mMessageHeader.hopCount + 1;

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

    for (HelloMessage::LinkMessage& msg : mLinkMessages) {
        // Link code
        PRINTLN(Seralized a link message)
        *(mSerializedData + vCurrentIndex) = msg.linkCode;
        vCurrentIndex++;

        // Reserved
        *(mSerializedData + vCurrentIndex) = 0;
        vCurrentIndex++;

        // Link msg size
        uint16_t msgSize = msg.neighborIfAddr.size() * WLAN_ADDR_LEN;
        *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(msgSize);
        vCurrentIndex += 2;

        for (auto& addr : msg.neighborIfAddr) {
            memcpy ( mSerializedData + vCurrentIndex, addr.data, WLAN_HEADER_LEN );
            vCurrentIndex += WLAN_HEADER_LEN;
        }
    }
}
