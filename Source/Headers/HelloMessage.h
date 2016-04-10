#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "IPv6Address.h"
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

	uint8_t getWillingness(){
		return willingness;
	}

	std::vector<LinkMessage> getLinkMessages() const{
		return linkMessages;
	}
private:
	uint8_t willingness;
	std::vector<LinkMessage> linkMessages;
};

#endif // HELLO_MESSAGE_H
