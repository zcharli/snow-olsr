#include "Headers/NetworkTrafficManager.h"

NetworkTrafficManager::NetworkTrafficManager(string interface)
    : mWirelessInterfaceName(interface) {
    mSemConsumer = new boost::interprocess::interprocess_semaphore(MIN_SEM);
    mSendSocket = make_shared<WLAN>(mWirelessInterfaceName);

}
NetworkTrafficManager::~NetworkTrafficManager() { }

void NetworkTrafficManager::init() {
    mListener = make_unique<NetworkPacketListener>(mWirelessInterfaceName, *this);
    mListener->run();
    mSendSocket->init();
    // Only create TC thread if this client is an MPR, lets test anyways
    mTCThread = make_unique<NetworkTCMessageThread>(mSendSocket);
    mHelloThread = make_unique<NetworkHelloMessageThread>(mSendSocket);
    mTCThread->run();
    mHelloThread->run();
    PRINTLN(Traffic manager has been initialized);
}

int NetworkTrafficManager::sendMsg(OLSRMessage& messageToSend) {
    //mSendSocket->send();
    return 0;
}

int NetworkTrafficManager::enqueMsgForProcessing() {
    mSemConsumer->wait();
    mMtxEnqueue.lock();
    //mReceivedMsgsQ.push_back()

    mListener->notifyProducerReady();
    mMtxEnqueue.unlock();
    return 1;
}

void NetworkTrafficManager::notifyConsumerReady() {
    mSemConsumer->post();
}

shared_ptr<OLSRMessage> NetworkTrafficManager::getMessage() {
    mMtxGetMessage.lock();
    shared_ptr<OLSRMessage> vMessage = mReceivedMsgsQ.front();
    mReceivedMsgsQ.pop();
    mMtxGetMessage.unlock();
    return vMessage;
}






/**
 * Message threads
 */

NetworkTCMessageThread::NetworkTCMessageThread(shared_ptr<WLAN> socket) : mSocket(socket) {

}

NetworkTCMessageThread::~NetworkTCMessageThread() {}

int NetworkTCMessageThread::run() {
    boost::thread tcMsgThread = boost::thread(&NetworkTCMessageThread::startBroadcastTCMessages, this);
    return 1;
}

void NetworkTCMessageThread::startBroadcastTCMessages() {
    PRINTLN(TC Message thread started);
}



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
