//
// Created by czl on 03/04/16.
//

#include "Headers/Packet.h"

Packet::Packet(char* src, char* dst, char* messageBuffer, int header_offset = 0)
    : mSource(src), mDestination(dst), mBuffer(messageBuffer), offset(header_offset) {
    mAddress = NULL;
    PRINTLN(Initialized a packet.)
}

Packet::Packet() {
}

Packet::~Packet() {
    PRINTLN(Destroying a packet does not destroy buffer.);
    delete mSource;
    delete mDestination;
    if(mAddress != NULL)
        delete mAddress;
    // delete mBuffer;
}

char* Packet::getSource(){
    return mSource;
}
char* Packet::getDestination() {
    return mDestination;
}
char* Packet::getBuffer() {
    return mBuffer;
}
char* Packet::getMyAddress() {
    return mAddress;
}
int Packet::getOffset() {
    return offset;
}
void Packet::setSource(char* data){
    mSource = data;
}
void Packet::setDestination(char* data){
    mDestination = data;
}
void Packet::setBuffer(char* data){
    mBuffer = data;
}
void Packet::setMyAddress(char* data) {
    mAddress = data;
}
void Packet::setOffset(int offset) {
    offset = offset;
}

