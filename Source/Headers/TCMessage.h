#ifndef TC_MESSAGE_H
#define TC_MESSAGE_H

#include <memory>
#include <vector>
#include "Message.h"
#include "IPv6Address.h"

class TCMessage : public Message
{
public:
	TCMessage();
	~TCMessage();


    uint16_t getAnsn()
    {
        return ansn;
    }

    std::vector<IPv6Address>& getNeighborAddresses(){
    	return neighborAddresses;
    }

private:
	uint16_t ansn;
	std::vector<IPv6Address> neighborAddresses;
};

#endif // TC_MESSAGE_H
