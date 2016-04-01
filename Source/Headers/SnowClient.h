#ifndef SNOW_CLIENT_H
#define SNOW_CLIENT_H

//#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <memory>
#include "WLAN.h"
#include "RoutingTable.h"
#include "NeighborNode.h"
#include "Resources/Constants.h"

using namespace std;

class SnowClient
{

private:
    RoutingTable mRoutingTable;
    //boost::asio::ip::udp::socket mSocket;
    unique_ptr<WLAN> mSocketPtr;
    std::vector<NeighborNode> mMprList;


public:
    SnowClient();
    ~SnowClient();
    int start();

};

#endif // SNOW_CLIENT_H
