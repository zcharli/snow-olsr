#include "Headers/HelloMessage.h"

HelloMessage::HelloMessage() {}
HelloMessage::~HelloMessage() {}

uint8_t HelloMessage::getWillingness() {
    std::cout << "HelloMessage::getWillingness not yet implemented" << std::endl;
    return 0;
}

std::vector<HelloMessage::LinkMessage> HelloMessage::getLinkMessages() const {
    return mLinkMessages;
}
