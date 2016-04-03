//
// Created by czl on 02/04/16.
//

#ifndef SNOW_OLSR_NETWORKPACKETLISTENER_H
#define SNOW_OLSR_NETWORKPACKETLISTENER_H

#include <iostream>
#include <string.h>
#include <memory>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "Resources/Helpers.h"
#include "Handler.h"
#include "WLAN.h"
#include "Packet.h"
#include "NetworkTrafficManager.h"

class NetworkTrafficManager;

class NetworkPacketListener : public Handler {
private:
    unique_ptr<WLAN> mRecvSocket;
    string mWirelessInterfaceName;
    NetworkTrafficManager& mMonitor;
    boost::mutex mMtxMonitor;
    boost::interprocess::interprocess_semaphore *mSemProducer;
    void listenOnInterface();
    void waitProducer();

public:
    NetworkPacketListener(string interface, NetworkTrafficManager& monitor);
    ~NetworkPacketListener();
    int run();
    virtual void handle(char src[], char dst[], char msg[]);
    void notifyProducerReady();
};


#endif //SNOW_OLSR_NETWORKPACKETLISTENER_H
