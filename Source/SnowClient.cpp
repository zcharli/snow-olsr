#include "Headers/SnowClient.h"
#include "Headers/NetworkTrafficManager.h"

/* Testing headers */
#include "Headers/HelloMessage.h"

using namespace std;

SnowClient::SnowClient() {
    //mSocketPtr = make_shared<WLAN>(INTERFACE_NAME);
}

SnowClient::~SnowClient() {}

int SnowClient::start() {
    shared_ptr<NetworkTrafficManager> vNetworkManager = make_shared<NetworkTrafficManager>(INTERFACE_NAME);
    vNetworkManager->init();
    RoutingProtocol::getInstance().setPersonalAddress(vNetworkManager->getPersonalAddress());

    while (1) {
        //PRINTLN(Snow client retrv msg);
        shared_ptr<Packet> vPacket = vNetworkManager->getMessage();
        if (vPacket != nullptr) {
            //std::cout << "First packet " << vPacket->getSource() << std::endl;
            if (vPacket->getSource()!= vNetworkManager->getPersonalAddress()) {
                std::cout << "Handle packet from " << vPacket->getSource() << std::endl;

                shared_ptr<OLSRMessage> vMessage = make_shared<OLSRMessage>(vPacket);
                std::cout << "Handle packet from " << vPacket->getSource() << std::endl;

                RoutingProtocol::getInstance().lockForUpdate();
                if(RoutingProtocol::getInstance().updateState(vMessage).needsForwarding()) {
                    vNetworkManager->sendMsg(*vMessage);
                }
                RoutingProtocol::getInstance().unLockAfterUpdate();
            } else {
                //std::cout << "Got a packet from my self lulz" << std::endl;
            }
        }
    }
    return 0;
}

// Code used to test deserialization and serialization
//
//         std::shared_ptr<HelloMessage> msg = make_shared<HelloMessage>();
//         HelloMessage::LinkMessage link;
//         link.linkCode = 255;
//         char a[] = {'1', '2', '3', '4', '5', '6'};
//         MACAddress linkAddr(a);
//         link.neighborIfAddr.push_back(linkAddr);
//         msg->mLinkMessages.push_back(link);
//         msg->type = 1;
//         msg->htime = 255;
//         msg->willingness = 255;
//         msg->mMessageHeader.type = 1;
//         msg->mMessageHeader.vtime = 255;
//         msg->mMessageHeader.messageSize = 8080;
//         memcpy(msg->mMessageHeader.originatorAddress, vNetworkManager->getPersonalAddress().data, WLAN_ADDR_LEN);
// // /msg.mMessageHeader.originatorAddress = vNetworkManager->getPersonalAddress().data;
//         msg->mMessageHeader.timeToLive = 255;
//         msg->mMessageHeader.hopCount = 255;
//         msg->mMessageHeader.messageSequenceNumber = 8080;

//         OLSRMessage omsg;
//         omsg.messages.push_back(msg);

//         char* serialized = omsg.serialize().getData();

//         OLSRMessage dmsg(serialized);
