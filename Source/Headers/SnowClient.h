#ifndef SNOW_CLIENT_H
#define SNOW_CLIENT_H

//#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <list>
#include "WLAN.h"
#include "RoutingTable.h"
#include "NeighborNode.h"
#include "Resources/Constants.h"

using namespace std;

class SnowClient
{

private:
    //boost::asio::ip::udp::socket mSocket;
    //shared_ptr<WLAN> mSocketPtr;
    list<NeighborNode> mMprList;
    string mWirelessInterfaceName;

public:
    SnowClient();
    ~SnowClient();
    int start();

};

#endif // SNOW_CLIENT_H
