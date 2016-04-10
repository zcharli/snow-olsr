#include "Headers/SnowClient.h"
#include "Headers/NetworkTrafficManager.h"

using namespace std;

SnowClient::SnowClient() {
    //mSocketPtr = make_shared<WLAN>(INTERFACE_NAME);
}

SnowClient::~SnowClient() {}

int SnowClient::start() {

    shared_ptr<NetworkTrafficManager> vNetworkManager = make_shared<NetworkTrafficManager>(INTERFACE_NAME);
    vNetworkManager->init();
    RoutingProtocol::getInstance().setPersonalAddress(vNetworkManager->getPersonalAddress());
    while(1) {
        PRINTLN(Snow client retrv msg);

        shared_ptr<Packet> vPacket = vNetworkManager->getMessage();
        shared_ptr<OLSRMessage> vMessage = make_shared<OLSRMessage>(vPacket);
        RoutingProtocol::getInstance().updateState(vMessage);
        vNetworkManager->notifyConsumerReady();
        sleep(3);
    }
    return 0;
}
