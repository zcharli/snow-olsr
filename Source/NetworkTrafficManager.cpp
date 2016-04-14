#include "Headers/NetworkTrafficManager.h"

NetworkTrafficManager::NetworkTrafficManager(std::string interface)
    : mWirelessInterfaceName(interface) {
    mSemConsumer = new boost::interprocess::interprocess_semaphore(MIN_SEM);
    mSendSocket = std::make_shared<WLAN>(mWirelessInterfaceName);

}
NetworkTrafficManager::~NetworkTrafficManager() {
    delete mSemConsumer;
}

void NetworkTrafficManager::init() {
    mListener = std::make_unique<NetworkPacketListener>(mWirelessInterfaceName, *this);
    mListener->run();
    mSendSocket->init();
    // Only create TC thread if this client is an MPR, lets test anyways
    mTCThread = std::make_unique<NetworkTCMessageThread>(mSendSocket);
    mHelloThread = std::make_unique<NetworkHelloMessageThread>(mSendSocket);
    mTCThread->run();
    mHelloThread->run();
    PRINTLN(Traffic manager has been initialized);
}

int NetworkTrafficManager::sendMsg(OLSRMessage& messageToSend) {
    //mSendSocket->send();
    return 0;
}

int NetworkTrafficManager::enqueMsgForProcessing(std::shared_ptr<Packet> packet) {
    mMtxMessageList.lock();
    mReceivedMsgsQ.push(packet);

    if(mReceivedMsgsQ.size() == 1) {
        mSemConsumer->post();
        //PRINTLN(Posted)
    }
    mMtxMessageList.unlock();
    return 1;
}

void NetworkTrafficManager::notifyConsumerReady() {
    mSemConsumer->post();
}

std::shared_ptr<Packet> NetworkTrafficManager::getMessage() {
    if(mReceivedMsgsQ.size() == 0) {
        //PRINTLN(Waiting)
        mSemConsumer->wait();
        //PRINTLN(Been posted)
    }
    mMtxMessageList.lock();
    if(mReceivedMsgsQ.size() == 0) {
        PRINTLN(Returned null msg)
        mMtxMessageList.unlock();
        return nullptr;
    }
    std::shared_ptr<Packet> vMessage = mReceivedMsgsQ.front();
    mReceivedMsgsQ.pop();
    mMtxMessageList.unlock();
    //mListener->notifyProducerReady();
    PRINTLN(Returned good msg)
    return vMessage;
}

const MACAddress& NetworkTrafficManager::getPersonalAddress() {
    return mSendSocket->getPersonalAddress();
}

int NetworkTrafficManager::generateRandomJitter() {
    boost::random::mt19937 vRandomSeed(time(0));
    boost::random::uniform_int_distribution<> vJitterGenerator(0,S_MAXJITTER);
    return vJitterGenerator(vRandomSeed);
}



/**
 * Message threads
 */

NetworkTCMessageThread::NetworkTCMessageThread(std::shared_ptr<WLAN> socket) : mSocket(socket) {

}

NetworkTCMessageThread::~NetworkTCMessageThread() {}

int NetworkTCMessageThread::run() {
    boost::thread tcMsgThread = boost::thread(&NetworkTCMessageThread::startBroadcastTCMessages, this);
    return 1;
}

void NetworkTCMessageThread::startBroadcastTCMessages() {
    PRINTLN(TC Message thread started);
    //int vPkgSeqNum = 1, vMsgSeqNum = 1; // Commented out cause unused atm.

    while (1) {
        char a[] = "ff:ff:ff:ff:ff:ff";
        //PRINTLN(Waiting to send)
        mSocketMutex.lock();
        OLSRMessage message;
        if(RoutingProtocol::getInstance().buildTCMessage(message) == 0) {
            std::cout << "Error when building TV message" << std::endl;
        }
        mSocketMutex.unlock();
        char* vBuffer = message.serialize().getData();
        //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
        //char f[] = "Hello!"; // data
        char* buffer = new char[message.getPacketSize() + WLAN_HEADER_LEN];
        memmove(buffer + WLAN_HEADER_LEN, vBuffer, message.getPacketSize());
        //buffer[message.getPacketSize() + 14] = '\0';
        mSocket->send(a, buffer);
        PRINTLN(Sent a TC message)
        //std::cout << "Sleeping for " << 1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()) << std::endl;
        usleep(1000*(T_TC_INTERVAL + NetworkTrafficManager::generateRandomJitter()));
        delete [] buffer;
    }
    PRINTLN(TC message thread closed down);
}



NetworkHelloMessageThread::NetworkHelloMessageThread(std::shared_ptr<WLAN> socket) : mSocket(socket) {

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
        //PRINTLN(Waiting to send)
        mSocketMutex.lock();
        OLSRMessage message;
        if(RoutingProtocol::getInstance().buildHelloMessage(message) == 0) {
            std::cout << "Error when building hello message" << std::endl;
        }
        mSocketMutex.unlock();
        char* vBuffer = message.serialize().getData();
        //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
        //char f[] = "Hello!"; // data
        char* buffer = new char[message.getPacketSize() + WLAN_HEADER_LEN];
        memmove(buffer + WLAN_HEADER_LEN, vBuffer, message.getPacketSize());
        //buffer[message.getPacketSize() + 14] = '\0';
        mSocket->send(a, buffer);
        PRINTLN(Sent a hello message)
        //std::cout << "Sleeping for " << 1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()) << std::endl;
        usleep(1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()));
        delete [] buffer;
    }
    PRINTLN(Hello message thread closed down);
}


// Garbage code dump
//
// int NetworkTrafficManager::enqueMsgForProcessing(std::shared_ptr<Packet> packet) {
//     //PRINTLN(EnQing)
//     mSemConsumer->wait();
//     //PRINTLN(been notified)
//     mMtxEnqueue.lock();
//     mReceivedMsgsQ.push(packet);

//     mListener->notifyProducerReady();
//     mMtxEnqueue.unlock();
//     return 1;
// }

// void NetworkTrafficManager::notifyConsumerReady() {
//     mSemConsumer->post();
// }

// std::shared_ptr<Packet> NetworkTrafficManager::getMessage() {

//     mMtxGetMessage.lock();
//     if(mReceivedMsgsQ.size() == 0) {
//         mMtxGetMessage.unlock();
//         return nullptr;
//     }
//     std::shared_ptr<Packet> vMessage = mReceivedMsgsQ.front();
//     mReceivedMsgsQ.pop();
//     mMtxGetMessage.unlock();
//     return vMessage;
// }
