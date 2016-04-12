#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <netinet/in.h>
#include <map>
#include <memory>
#include "IPv6Address.h"
#include "Message.h"
#include "Resources/Helpers.h"

class HelloMessage : public Message
{

public:
    struct LinkMessage {
        uint8_t linkCode;
        std::vector<IPv6Address> neighborIfAddr;
    };
    HelloMessage();
    HelloMessage(char*);
    ~HelloMessage();

    void serialize();
    void deserialize(char*);
    uint8_t getWillingness();
    std::vector<LinkMessage> mLinkMessages;
	std::vector<LinkMessage> getLinkMessages() const;
    uint8_t htime;

private:
    uint8_t willingness;
};

#endif // HELLO_MESSAGE_H
