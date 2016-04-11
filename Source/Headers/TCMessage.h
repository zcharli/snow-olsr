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

	std::vector<int> serialize();

	
	uint16_t getAnsn();
	std::vector<IPv6Address> mNeighborAddresses;
	
private:
	uint16_t ansn;
};

#endif // TC_MESSAGE_H
