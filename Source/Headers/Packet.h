//
// Created by czl on 03/04/16.
//

#ifndef SNOW_OLSR_PACKET_H
#define SNOW_OLSR_PACKET_H

#include <string>
#include <iostream>
#include <array>
#include "Resources/Helpers.h"
#include "MACAddress.h"

#define ADDRESS_SIZE 32

//using namespace std;

class Packet {
public:
    Packet(char*, char*, char*, int);
    Packet();
    ~Packet();
    MACAddress& getSource();
    MACAddress& getDestination();
    char* getBuffer();
    char* getMyAddress();
    int getOffset();
    void setSource(MACAddress);
    void setDestination(MACAddress);
    void setBuffer(char*);
    void setMyAddress(char*);
    void setOffset(int);

private:
    MACAddress mSource;
    MACAddress mDestination;
    char* mBuffer;
    char* mAddress;
    int offset;
};


#endif //SNOW_OLSR_PACKET_H
