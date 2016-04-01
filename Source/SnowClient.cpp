#include "Headers/SnowClient.h"

using namespace std;

SnowClient::SnowClient() {
    mSocketPtr = make_unique<WLAN>(INTERFACE_NAME);
}
SnowClient::~SnowClient() {}

int SnowClient::start() {

    return 0;
}
