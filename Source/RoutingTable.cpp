#include "Headers/RoutingTable.h"

RoutingTable& RoutingTable::getInstance() {
    static RoutingTable instance;
    return instance;
}

