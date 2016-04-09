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
        PRINTLN(Snow client keeping itself alive);
        sleep(3);
        vNetworkManager->notifyConsumerReady();
    }
    return 0;
}
