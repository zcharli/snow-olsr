//
// Created by czl on 03/04/16.
//

#ifndef SNOW_OLSR_NETWORKHELLOMESSAGETHREAD_H
#define SNOW_OLSR_NETWORKHELLOMESSAGETHREAD_H

#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "WLAN.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"
#include "HelloMessage.h"

class NetworkHelloMessageThread {
public:
    NetworkHelloMessageThread(shared_ptr<WLAN> socket);
    ~NetworkHelloMessageThread();
    int run();
private:
    void startBroadcastHelloMessages();
    boost::mutex mSocketMutex;
    shared_ptr<WLAN> mSocket;
};


#endif //SNOW_OLSR_NETWORKHELLOMESSAGETHREAD_H
