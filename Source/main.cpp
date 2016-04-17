#include <iostream>
#include "Headers/SnowClient.h"
#include "Headers/Resources/Helpers.h"
using namespace std;

/**
 * Boot Procedure (Need verification)
 *  1. Enable /proc/sys/net/ipv6/conf/<interface or all> - set to 1
 *
 * @return exit
 */

int main(int argc, char* argv[])
{
    SnowClient snow;
    #if verbose
    PRINTLN(SnowClient has started.);
    #endif
    snow.start();

    return 0;
}
