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


//using namespace std;

class NetworkPacketListener;
class NetworkTCMessageThread;
class NetworkHelloMessageThread;

class NetworkTrafficManager
{
private:
    void send();
    std::unique_ptr<NetworkPacketListener> mListener;
    std::shared_ptr<WLAN> mSendSocket;
    boost::mutex mMtxMessageList, mMtxEnqueue, mMtxGetMessage;
    std::queue<std::shared_ptr<Packet>> mReceivedMsgsQ;
    std::string mWirelessInterfaceName;
    std::unique_ptr<NetworkTCMessageThread> mTCThread;
    std::unique_ptr<NetworkHelloMessageThread> mHelloThread;
    boost::interprocess::interprocess_semaphore *mSemConsumer;

public:
    NetworkTrafficManager(std::string interface);
    ~NetworkTrafficManager();
    int sendMsg(OLSRMessage& message);
    void init();
    int enqueMsgForProcessing(std::shared_ptr<Packet> );
    void notifyConsumerReady();
    std::shared_ptr<Packet> getMessage();
    const IPv6Address& getPersonalAddress();
    static int generateRandomJitter();

};

class NetworkTCMessageThread {
public:
    NetworkTCMessageThread(std::shared_ptr<WLAN> socket);
    ~NetworkTCMessageThread();
    int run();
private:
    void startBroadcastTCMessages();
    boost::mutex mSocketMutex;
    std::shared_ptr<WLAN> mSocket;
};


class NetworkHelloMessageThread {
public:
    NetworkHelloMessageThread(std::shared_ptr<WLAN> socket);
    ~NetworkHelloMessageThread();
    int run();
private:
    void startBroadcastHelloMessages();
    boost::mutex mSocketMutex;
    std::shared_ptr<WLAN> mSocket;
};




#endif // NETWORK_TRAFFIC_MANAGER_H
