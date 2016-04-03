//
// Created by czl on 03/04/16.
//

#include "Headers/NetworkTCMessageThread.h"

NetworkTCMessageThread::NetworkTCMessageThread(shared_ptr<WLAN> socket) : mSocket(socket) {

}

NetworkTCMessageThread::~NetworkTCMessageThread() {}

int NetworkTCMessageThread::run() {
    boost::thread tcMsgThread = boost::thread(&NetworkTCMessageThread::startBroadcastTCMessages, this);
    return 1;
}

void NetworkTCMessageThread::startBroadcastTCMessages() {

}
