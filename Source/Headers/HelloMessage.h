#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <memory>
#include "MACAddress.h"
#include "Message.h"
#include "Resources/Helpers.h"

class HelloMessage : public Message
{

public:
    struct LinkMessage {
        uint8_t linkCode;
        std::vector<MACAddress> neighborIfAddr;
    };
    HelloMessage();
    HelloMessage(char*);
    virtual ~HelloMessage();

    virtual void serialize();
    virtual void deserialize(char*);
    uint8_t getWillingness();
    std::vector<LinkMessage> mLinkMessages;
	std::vector<LinkMessage> getLinkMessages() const;
    uint8_t htime;
    uint8_t willingness;

private:

};

#endif // HELLO_MESSAGE_H
