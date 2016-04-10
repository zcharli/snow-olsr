#ifndef SNOW_CLIENT_H
#define SNOW_CLIENT_H

//#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <list>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "WLAN.h"
#include "RoutingProtocol.h"
#include "NeighborNode.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"


//using namespace std;

class SnowClient
{

private:
    //boost::asio::ip::udp::socket mSocket;
    //shared_ptr<WLAN> mSocketPtr;
    std::list<NeighborNode> mMprList;
    std::string mWirelessInterfaceName;

public:
    SnowClient();
    ~SnowClient();
    int start();

};

#endif // SNOW_CLIENT_H
