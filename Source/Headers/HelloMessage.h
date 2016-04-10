#ifndef HELLO_MESSAGE_H
#define HELLO_MESSAGE_H

<<<<<<< HEAD
#include <iostream>
#include <vector>
=======
#include <vector>
#include <map>
#include <memory>
#include "IPv6Address.h"
>>>>>>> RoutingProtocol Update
#include "Message.h"
#include "IPv6Address.h"

class HelloMessage : public Message
{

public:
<<<<<<< HEAD
    struct LinkMessage {
        uint8_t linkCode;
        std::vector<IPv6Address> neighborIfAddr;
    };
    HelloMessage();
    ~HelloMessage();
    std::vector<LinkMessage> mLinkMessages;
=======
	struct  LinkMessage{
		uint8_t linkCode;
		vector<IPv6Address> neighborInterfaceAddresses;
	};
	HelloMessage();
	~HelloMessage();
>>>>>>> RoutingProtocol Update

	uint8_t getWillingness(){
		return willingness;
	}

	vector<LinkMessage> getLinkMessages() const{
		return linkMessages;
	}
private:
	uint8_t willingness;
	vector<LinkMessage> linkMessages;
};

#endif // HELLO_MESSAGE_H
