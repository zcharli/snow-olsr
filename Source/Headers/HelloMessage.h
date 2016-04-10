#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

#include <iostream>
#include <vector>
#include "Message.h"
#include "IPv6Address.h"

class HelloMessage : public Message
{

public:
    struct LinkMessage {
        uint8_t linkCode;
        std::vector<IPv6Address> neighborIfAddr;
    };
    HelloMessage();
    ~HelloMessage();
    std::vector<LinkMessage> mLinkMessages;

};

#endif // HELLO_MESSAGE_H
