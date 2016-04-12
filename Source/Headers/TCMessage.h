#ifndef TC_MESSAGE_H
#define TC_MESSAGE_H

#include <iostream>
#include <stdint.h>
#include <memory>
#include <vector>
#include "IPv6Address.h"
#include "Message.h"

class TCMessage : public Message
{
public:
	TCMessage();
    TCMessage(char*);
	~TCMessage();

	virtual void serialize();
    virtual void deserialize(char*);

	uint16_t getAnsn();
	std::vector<IPv6Address> mNeighborAddresses;

private:
	uint16_t ansn;
};

#endif // TC_MESSAGE_H
