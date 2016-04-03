//
// Created by czl on 03/04/16.
//

#include "Headers/NetworkHelloMessageThread.h"

NetworkHelloMessageThread::NetworkHelloMessageThread(shared_ptr<WLAN> socket) : mSocket(socket) {

}

NetworkHelloMessageThread::~NetworkHelloMessageThread() {}

int NetworkHelloMessageThread::run() {
    boost::thread helloMsgThread = boost::thread(&NetworkHelloMessageThread::startBroadcastHelloMessages, this);
    return 1;
}

void NetworkHelloMessageThread::startBroadcastHelloMessages() {
    PRINTLN(Hello Message thread started)
    //int vPkgSeqNum = 1, vMsgSeqNum = 1; // Commented out cause unused atm.
    while (1) {
        char a[] = "ff:ff:ff:ff:ff:ff";
        //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
        char f[] = "Hello!"; // data

        int array_size = strlen(f);
        // Make a buffer greater than
        char* buffer = (char*)malloc( array_size + 14);

        memmove(buffer + 14, f, array_size);
        buffer[array_size + 14] = '\0';
        mSocket->send(a, buffer);
        PRINTLN(Sent a hello message)
        sleep(T_HELLO_INTERVAL);
    }
    PRINTLN(Hello message thread closed down);
}
