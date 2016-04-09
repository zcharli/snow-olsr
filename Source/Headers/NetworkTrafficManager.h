#ifndef NETWORK_TRAFFIC_MANAGER_H
#define NETWORK_TRAFFIC_MANAGER_H

#include <iostream>
#include <string.h>
#include <queue>
#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "WLAN.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"
#include "OLSRMessage.h"
#include "NetworkPacketListener.h"
#include "NetworkTCMessageThread.h"
#include "NetworkHelloMessageThread.h"


using namespace std;

class NetworkPacketListener;

class NetworkTrafficManager
{
private:
    void send();
    unique_ptr<NetworkPacketListener> mListener;
    shared_ptr<WLAN> mSendSocket;
    boost::mutex mMtxMessageList, mMtxEnqueue, mMtxGetMessage;
    queue<shared_ptr<OLSRMessage>> mReceivedMsgsQ;
    string mWirelessInterfaceName;
    unique_ptr<NetworkTCMessageThread> mTCThread;
    unique_ptr<NetworkHelloMessageThread> mHelloThread;
    boost::interprocess::interprocess_semaphore *mSemConsumer;

public:
    NetworkTrafficManager(string interface);
    ~NetworkTrafficManager();
    int sendMsg(OLSRMessage& message);
    void init();
    int enqueMsgForProcessing();
    void notifyConsumerReady();
    shared_ptr<OLSRMessage> getMessage();


};

#endif // NETWORK_TRAFFIC_MANAGER_H