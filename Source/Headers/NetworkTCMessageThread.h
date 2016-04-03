//
// Created by czl on 03/04/16.
//

#ifndef SNOW_OLSR_NETWORKTCMESSAGETHREAD_H
#define SNOW_OLSR_NETWORKTCMESSAGETHREAD_H

#include <iostream>
#include <memory>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "WLAN.h"
#include "Resources/Constants.h"

class NetworkTCMessageThread {
public:
    NetworkTCMessageThread(shared_ptr<WLAN> socket);
    ~NetworkTCMessageThread();
    int run();
private:
    void startBroadcastTCMessages();
    boost::mutex mSocketMutex;
    shared_ptr<WLAN> mSocket;
};


#endif //SNOW_OLSR_NETWORKTCMESSAGETHREAD_H
