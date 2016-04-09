//
// Created by czl on 02/04/16.
//

#include "Headers/NetworkPacketListener.h"

NetworkPacketListener::NetworkPacketListener(string interface, NetworkTrafficManager& monitor)
    : mWirelessInterfaceName(interface), mMonitor(monitor) {
    mRecvSocket = make_unique<WLAN>(mWirelessInterfaceName);
    mRecvSocket->setHandler(this); // This is essentially debugging
    mRecvSocket->init();
    mSemProducer = new boost::interprocess::interprocess_semaphore(MAX_SEM);
}

NetworkPacketListener::~NetworkPacketListener() {}

int NetworkPacketListener::run() {
    boost::thread blockingRecvThread = boost::thread(&NetworkPacketListener::listenOnInterface, this);
    return 1;
}

void NetworkPacketListener::listenOnInterface() {
    PRINTLN(Network Packet Listener thread started);
    while(1){
        PRINTLN(Semaphore wait)
        mSemProducer->wait(); // Wait for the signal from Manager class
        shared_ptr<Packet> vPacket = make_shared<Packet>();
        mRecvSocket->receive(vPacket);
        // Received a packet.  Now fucking make me a message
        PRINTLN(Received a packet);
        mMtxMonitor.lock();
        PRINTLN(Inside lock);
        // The following block must be resolved by the consumer
        //if(mMonitor.enqueMsgForProcessing() == 1) {} // Forcing a wait when we dont want it
        mMtxMonitor.unlock();
        PRINTLN(Lock resolved)
        // End of transaction, then tell consume to eat.
        mMonitor.notifyConsumerReady();
        PRINTLN(Monitor notified);
    }

}

void NetworkPacketListener::handle(char src[], char dst[], char msg[]) {
    //PRINTLN(from  << src << to  << dst << :  << msg)
    cout << "from " << src << " to " << dst << ": " << msg << "\n";

}
void NetworkPacketListener::notifyProducerReady() {
    mSemProducer->post(); // Green light for socket to receive more
}