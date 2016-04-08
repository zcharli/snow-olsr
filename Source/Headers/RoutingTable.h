#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

// OLSR Type
#include "Resources/Types.h"

using namespace std;
using namespace olsr;

/*
 Singleton Static class
 */
class RoutingTable
{
protected:
    MprSet                  mprSet;
    MprSelectorSet          mprSelectorSet;
    LinkSet                 linkSet;
    NeighborSet             neighborSet;
    TwoHopNeighborSet       twoHopNeighborSet;
    TopologySet             topologySet;
    InterfaceAssociationSet interfaceAssociationSet;

public:
    RoutingTable() {}
    RoutingTable(RoutingTable const&);
    void operator=(RoutingTable const&);
    static RoutingTable& getInstance();

    // Getter:
    const MprSelectorSet & GetMprSelectors () const             { return mprSelectorSet;            }
    const LinkSet & GetLinks () const                           { return linkSet;                   }
    const NeighborSet & GetNeighbors () const                   { return neighborSet;               }
    NeighborSet & GetNeighbors ()                               { return neighborSet;               }
    const TwoHopNeighborSet & GetTwoHopNeighbors () const       { return twoHopNeighborSet;         }
    TwoHopNeighborSet & GetTwoHopNeighbors ()                   { return twoHopNeighborSet;         }
    const TopologySet & GetTopologySet () const                 { return topologySet;               }
    const InterfaceAssociationSet & GetIfaceAssocSet () const   { return interfaceAssociationSet;   }
    InterfaceAssociationSet & GetIfaceAssocSetMutable ()        { return interfaceAssociationSet;   }

    // MPR
    bool FindMprAddress (const Ipv4Address  &address);
    void SetMprSet (MprSet mprSet);

    // MPR selector
    MprSelectorTuple* FindMprSelectorTuple (const Ipv4Address &mainAddr);
    void CleanMprSelectorTuple (const MprSelectorTuple &tuple);
    void CleanMprSelectorTuples (const Ipv4Address &mainAddr);
    void InsertMprSelectorTuple (const MprSelectorTuple &tuple);
    std::string PrintMprSelectorSet () const;

    // Link
    LinkTuple* FindLinkTuple (const Ipv4Address &ifaceAddr);
    LinkTuple* FindSymLinkTuple (const Ipv4Address &ifaceAddr);
    void CleanLinkTuple (const LinkTuple &tuple);
    LinkTuple& InsertLinkTuple (const LinkTuple &tuple);

    // Neighbor
    NeighborTuple* FindNeighborTuple (const Ipv4Address &mainAddr);
    const NeighborTuple* FindSymNeighborTuple (const Ipv4Address &mainAddr) const;
    NeighborTuple* FindNeighborTuple (const Ipv4Address &mainAddr, uint8_t willingness);
    void CleanNeighborTuple (const NeighborTuple &neighborTuple);
    void CleanNeighborTuple (const Ipv4Address &mainAddr);
    void InsertNeighborTuple (const NeighborTuple &tuple);

    // Two-hop neighbor
    TwoHopNeighborTuple* FindTwoHopNeighborTuple (const Ipv4Address &neighbor, const Ipv4Address &twoHopNeighbor);
    void CleanTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);
    void CleanTwoHopNeighborTuples (const Ipv4Address &neighbor);
    void CleanTwoHopNeighborTuples (const Ipv4Address &neighbor, const Ipv4Address &twoHopNeighbor);
    void InsertTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);

    // Topology
    TopologyTuple* FindTopologyTuple (const Ipv4Address &destAddr, const Ipv4Address &lastAddr);
    TopologyTuple* FindNewerTopologyTuple (const Ipv4Address &lastAddr, uint16_t ansn);
    void CleanTopologyTuple (const TopologyTuple &tuple);
    void CleanOlderTopologyTuples (const Ipv4Address &lastAddr, uint16_t ansn);
    void InsertTopologyTuple (const TopologyTuple &tuple);

    // Interface association
    InterfaceAssociationTuple* FindInterfaceAssociationTuple (const Ipv4Address &ifaceAddr);
    const InterfaceAssociationTuple* FindInterfaceAssociationTuple (const Ipv4Address &ifaceAddr) const;
    void CleanInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);
    void InsertInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);

    // Returns a vector of all interfaces of a given neighbor, with the
    // exception of the "main" one.
    std::vector<Ipv4Address> FindNeighborInterfaces (const Ipv4Address &neighborMainAddr) const;
};

#endif // ROUTING_TABLE_H
