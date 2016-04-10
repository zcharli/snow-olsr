#ifndef TC_MESSAGE_H
#define TC_MESSAGE_H

#include <stdint.h>
#include <memory>
#include <vector>
#include "Message.h"
#include "IPv6Address.h"

class Message;

class TCMessage : public Message
{
public:
	TCMessage();
	~TCMessage();

	char* serialize();

private:
	uint16_t ansn;
	std::vector<IPv6Address> mNeighborAddresses;
};

#endif // TC_MESSAGE_H
