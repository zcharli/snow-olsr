#ifndef SNOW_CLIENT_H
#define SNOW_CLIENT_H

//#include <boost/asio.hpp>
#include <string>
#include <stdint.h>
#include <iostream>
#include <memory>
#include <list>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "WLAN.h"
#include "RoutingProtocol.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"


//using namespace std;

class SnowClient
{

private:
    std::string mWirelessInterfaceName;

public:
    SnowClient();
    ~SnowClient();
    int start();

};

#endif // SNOW_CLIENT_H
