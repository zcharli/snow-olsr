#include "Headers/TCMessage.h"

TCMessage::TCMessage() {
    mMessageHeader.type = M_TC_MESSAGE;
}
TCMessage::~TCMessage() {
}

TCMessage::TCMessage(char* buffer) {
    mMessageHeader.type = M_TC_MESSAGE;
    deserialize(buffer);
}

uint16_t TCMessage::getAnsn() {
    return ansn;
}

void TCMessage::deserialize(char* buffer) {
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
    std::cout << "make_shared MACAddress TC message" << std::endl;

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
    // Message sequence number
    ansn = ntohs((*(uint16_t*) buffer));
    buffer += 2;

    // Skip the reserved
    buffer += 2;

    while (vTotalMsgSize > 0) {
        char vAdvertisedNeighborInterfaceAddrBuffer[WLAN_ADDR_LEN];
        memcpy(vAdvertisedNeighborInterfaceAddrBuffer, buffer, WLAN_ADDR_LEN);
        MACAddress vAdvertisedNeighborInterfaceAddr(vAdvertisedNeighborInterfaceAddrBuffer);
        mNeighborAddresses.push_back(vAdvertisedNeighborInterfaceAddr);
        buffer += WLAN_ADDR_LEN;
        vTotalMsgSize -= WLAN_ADDR_LEN;
        std::cout << "del [] buffer tc vAdvertisedNeighborInterfaceAddrBuffer: " << vAdvertisedNeighborInterfaceAddr << std::endl;
    }

}

void TCMessage::serialize() {
    if (mSerializedData != nullptr) {
        std::cout << "Found a non nullptr serialize data, deleting it" << std::endl;
        delete [] mSerializedData;
        mSerializedData = nullptr;
        mSerializedDataSize = 0;
    }
    int vCurrentIndex = 0;
    mSerializedDataSize = HELLO_MSG_HEADER;

    mSerializedDataSize += 4; // ANSN + reserved
    mSerializedDataSize += mNeighborAddresses.size() * WLAN_ADDR_LEN;
    std::cout << "This is the neighbor addr size" << mNeighborAddresses.size() * WLAN_ADDR_LEN << std::endl;

    mSerializedData = new char[mSerializedDataSize];

    // Make header
    // MsgType
    mSerializedData[vCurrentIndex++] = mMessageHeader.type;

    // VTime
    mSerializedData[vCurrentIndex++] = mMessageHeader.vtime;

    // MessageSize
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(mSerializedDataSize - HELLO_MSG_HEADER); // 22 is header size

    // Test
    uint16_t test = ntohs((*(uint16_t*) (mSerializedData + vCurrentIndex)));
    std::cout << "This is the size of the tc message" << test << std::endl;
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

    // Make TC msg
    *(uint16_t*)(mSerializedData + vCurrentIndex) = htons(ansn); // 22 is header size
    vCurrentIndex += 2;

    // Reserved section
    mSerializedData[vCurrentIndex++] = 0;
    mSerializedData[vCurrentIndex++] = 0;

    // Advertised mpr selectors
    for (auto& macAddresses : mNeighborAddresses) {
        memcpy ( mSerializedData + vCurrentIndex, macAddresses.data, WLAN_HEADER_LEN );
        vCurrentIndex += WLAN_HEADER_LEN;
    }

}
