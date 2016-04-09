#ifndef NETWORK_TRAFFIC_MANAGER_H
#define NETWORK_TRAFFIC_MANAGER_H

#include <iostream>
#include <string.h>
#include <queue>
#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
// JITTER GENERATOR
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "WLAN.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"
#include "OLSRMessage.h"
#include "NetworkPacketListener.h"
#include "IPv6Address.h"
#include "Packet.h"
//#include "NetworkTCMessageThread.h"
//#include "NetworkHelloMessageThread.h"


using namespace std;

class NetworkPacketListener;
class NetworkTCMessageThread;
class NetworkHelloMessageThread;

class NetworkTrafficManager
{
private:
    void send();
    unique_ptr<NetworkPacketListener> mListener;
    shared_ptr<WLAN> mSendSocket;
    boost::mutex mMtxMessageList, mMtxEnqueue, mMtxGetMessage;
    queue<shared_ptr<Packet>> mReceivedMsgsQ;
    string mWirelessInterfaceName;
    unique_ptr<NetworkTCMessageThread> mTCThread;
    unique_ptr<NetworkHelloMessageThread> mHelloThread;
    boost::interprocess::interprocess_semaphore *mSemConsumer;

public:
    NetworkTrafficManager(string interface);
    ~NetworkTrafficManager();
    int sendMsg(OLSRMessage& message);
    void init();
    int enqueMsgForProcessing(shared_ptr<Packet> );
    void notifyConsumerReady();
    shared_ptr<Packet> getMessage();
    const IPv6Address& getPersonalAddress();
    static int generateRandomJitter();

};

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




#endif // NETWORK_TRAFFIC_MANAGER_H
