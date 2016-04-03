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
