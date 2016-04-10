#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "IPv6Address.h"
#include "Message.h"

class HelloMessage : public Message
{

public:
    struct LinkMessage {
        uint8_t linkCode;
        std::vector<IPv6Address> neighborIfAddr;
    };
    HelloMessage();
    ~HelloMessage();

    std::vector<int> serialize();
    uint8_t getWillingness();
    std::vector<LinkMessage> mLinkMessages;
	std::vector<LinkMessage> getLinkMessages() const;

private:
	uint8_t willingness;
};

#endif // HELLO_MESSAGE_H
