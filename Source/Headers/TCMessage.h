#ifndef TC_MESSAGE_H
#define TC_MESSAGE_H

#include <iostream>
#include <stdint.h>
#include <memory>
#include <vector>
#include "MACAddress.h"
#include "Message.h"

class TCMessage : public Message
{
public:
    TCMessage();
    TCMessage(char*);
    virtual ~TCMessage();

    virtual void serialize();
    virtual void deserialize(char*);

    uint16_t getAnsn();
    std::vector<MACAddress> mNeighborAddresses;
    uint16_t ansn;
private:

};

#endif // TC_MESSAGE_H
