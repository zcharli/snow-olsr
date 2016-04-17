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
    #if verbose
    PRINTLN(Traffic manager has been initialized);
    #endif
}

int NetworkTrafficManager::sendMsg(std::shared_ptr<OLSRMessage> message) {
    char a[] = "ff:ff:ff:ff:ff:ff";
    #if verbose
    PRINTLN(Forwarding TC message)
    #endif
    // if (RoutingProtocol::getInstance().buildTCMessage(message) == 0) {
    //     std::cout << "Error when building TV message" << std::endl;
    // }
    message->serialize();
    //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
    //char f[] = "Hello!"; // data
    int size = message->getPacketSize() + WLAN_HEADER_LEN;
    char buffer[MAX_BUF];

    memcpy(buffer + WLAN_HEADER_LEN, message->getData(), message->getPacketSize());
    //buffer[message.getPacketSize() + 14] = '\0';
    mSendSocket->send(a, buffer, size);
    #if verbose
    PRINTLN(TC message forwarding was successful);
    #endif
    //std::cout << "Sleeping for " << 1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()) << std::endl;
    //std::cout << "del [] buffer sen msg of <<<<<<<<<<<<<<<<<<<<<<<<<<<< " <<  size << std::endl;
    return 0;
}

int NetworkTrafficManager::enqueMsgForProcessing(std::shared_ptr<Packet> packet) {
    mMtxMessageList.lock();
    mReceivedMsgsQ.push(packet);

    if (mReceivedMsgsQ.size() == 1) {
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
    if (mReceivedMsgsQ.size() == 0) {
        //PRINTLN(Waiting)
        mSemConsumer->wait();
        //PRINTLN(Been posted)
    }
    mMtxMessageList.lock();
    if (mReceivedMsgsQ.size() == 0) {
        #if verbose
        PRINTLN(Returned null msg)
        #endif
        mMtxMessageList.unlock();
        return nullptr;
    }
    std::shared_ptr<Packet> vMessage = mReceivedMsgsQ.front();
    mReceivedMsgsQ.pop();
    mMtxMessageList.unlock();
    //std::cout << "The producer has produced " << mReceivedMsgsQ.size() << " elements so far" << std::endl;
    //mListener->notifyProducerReady();
    return vMessage;
}

const MACAddress& NetworkTrafficManager::getPersonalAddress() {
    return mSendSocket->getPersonalAddress();
}

int NetworkTrafficManager::generateRandomJitter() {
    boost::random::mt19937 vRandomSeed(time(0));
    boost::random::uniform_int_distribution<> vJitterGenerator(0, S_MAXJITTER);
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
    #if verbose
    PRINTLN(TC Message thread started);
    #endif
    //int vPkgSeqNum = 1, vMsgSeqNum = 1; // Commented out cause unused atm.
    char a[] = "ff:ff:ff:ff:ff:ff";
    while (1) {

        //PRINTLN(Waiting to send)
        mSocketMutex.lock();
        OLSRMessage message;

        if (RoutingProtocol::getInstance().buildTCMessage(message) == 0) {
            // std::cout << "Error when building TV message" << std::endl;
        }
        mSocketMutex.unlock();
        message.serialize();
        //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
        //char f[] = "Hello!"; // data
        int size = message.getPacketSize() + WLAN_HEADER_LEN;
        char buffer[MAX_BUF];
        memcpy(buffer + WLAN_HEADER_LEN, message.getData(), message.getPacketSize());
        //buffer[message.getPacketSize() + 14] = '\0';
        mSocket->send(a, buffer, size);
        #if verbose
        PRINTLN(Sent a TC message)
        #endif
        //std::cout << "Sleeping for " << 1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()) << std::endl;
        usleep(1000 * (T_TC_INTERVAL + NetworkTrafficManager::generateRandomJitter()));
    }
    #if verbose
    PRINTLN(TC message thread closed down);
    #endif
}



NetworkHelloMessageThread::NetworkHelloMessageThread(std::shared_ptr<WLAN> socket) : mSocket(socket) {

}

NetworkHelloMessageThread::~NetworkHelloMessageThread() {}

int NetworkHelloMessageThread::run() {
    boost::thread helloMsgThread = boost::thread(&NetworkHelloMessageThread::startBroadcastHelloMessages, this);
    return 1;
}

void NetworkHelloMessageThread::startBroadcastHelloMessages() {
    #if verbose
    PRINTLN(Hello Message thread started)
    #endif
    //int vPkgSeqNum = 1, vMsgSeqNum = 1; // Commented out cause unused atm.
    char a[] = "ff:ff:ff:ff:ff:ff";
    while (1) {
        //PRINTLN(Waiting to send)
        mSocketMutex.lock();
        OLSRMessage message;
        if (RoutingProtocol::getInstance().buildHelloMessage(message) == 0) {
            // std::cout << "Error when building hello message" << std::endl;
        }
        mSocketMutex.unlock();
        message.serialize();
        //char a[]="1c:bd:b9:7e:b5:d4"; // unicast address
        //char f[] = "Hello!"; // data
        int size = message.getPacketSize() + WLAN_HEADER_LEN;
        //std::cout << "del [] buffer hello" << std::endl;
        char buffer[MAX_BUF];
        memcpy(buffer + WLAN_HEADER_LEN, message.getData(), message.getPacketSize());
        //buffer[message.getPacketSize() + 14] = '\0';
        mSocket->send(a, buffer, size);
        #if verbose
        PRINTLN(Sent a hello message)
        #endif
        //std::cout << "Sleeping for " << 1000*(T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()) << std::endl;
        usleep(1000 * (T_HELLO_INTERVAL + NetworkTrafficManager::generateRandomJitter()));
        //std::cout << "del [] buffer hello" << std::endl;
    }
    #if verbose
    PRINTLN(Hello message thread closed down);
    #endif
}
