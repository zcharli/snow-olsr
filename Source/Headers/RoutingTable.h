#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

using namespace std;
/*
 Singleton Static class
 */
class RoutingTable
{

public:
    static RoutingTable& getInstance();

private:
    RoutingTable() {};
    RoutingTable(RoutingTable const&);
    void operator=(RoutingTable const&);
};

#endif // ROUTING_TABLE_H
