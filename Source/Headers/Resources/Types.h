#ifndef TYPE_H
#define TYPE_H

#include <set>
#include <vector>

// Using ipv4-address???
// Using NS3? or peplace boots???
//#include "boost/asio/ip/address.hpp"
//#include "ns3/ipv4-address.h"
//#include "ns3/nstime.h"
//#include "boost/asio.hpp"
//#include <ctime>
#include "../IPv6Address.h"
/*
    Introduction of Type.h for OLSR

    Type Name                   Description                         Data Type
    MprSet:                     MPR Set type                        std::set<IPv6Address>
    MprSelectorSet:             MPR Selector Set type               std::vector<MprSelectorTuple>
    LinkSet:                    Link Set type                       std::vector<LinkTuple>
    NeighborSet:                Neighbor Set type                   std::vector<NeighborTuple>
    TwoHopNeighborSet:          2-hop Neighbor Set type             std::vector<TwoHopNeighborTuple>
    TopologySet:                Topology Set type                   std::vector<TopologyTuple>
    DuplicateSet:               Duplicate Set type                  std::vector<DuplicateTuple>
    InterfaceAssociationSet:    Interface Association Set type      std::vector<InterfaceAssociationTuple>

    Helper Function of operator == for
        MprSelectorTuple
        LinkTuple
        NeighborTuple
        TwoHopNeighborTuple
        TopologyTuple
        InterfaceAssociationTuple

    Helper Function of operator << for
        LinkTuple
        NeighborTuple
        TwoHopNeighborTuple
        TopologyTuple
        InterfaceAssociationTuple
*/

//#define IPv6Address char

/// An MPR-Selector Tuple.
struct MprSelectorTuple
{
    /// Main address of a node which have selected this node as a MPR.
    IPv6Address mainAddr;
    /// Time at which this tuple expires and must be removed.
};

static inline bool operator == (const MprSelectorTuple &a, const MprSelectorTuple &b)
{
    return (a.mainAddr == b.mainAddr);
}

/// A Link Tuple.
struct LinkTuple
{
    /// Interface address of the local node.
    IPv6Address localIfaceAddr;
    /// Interface address of the neighbor node.
    IPv6Address neighborIfaceAddr;
};

static inline bool operator == (const LinkTuple &a, const LinkTuple &b)
{
    return (a.localIfaceAddr == b.localIfaceAddr && a.neighborIfaceAddr == b.neighborIfaceAddr);
}

// static inline std::ostream& operator << (std::ostream &os, const LinkTuple &tuple)
// {
//     os  << "LinkTuple(localIfaceAddr=" << tuple.localIfaceAddr
//         << ", neighborIfaceAddr=" << tuple.neighborIfaceAddr;
//     return os;
// }

/// A Neighbor Tuple.
struct NeighborTuple
{
    /// Main address of a neighbor node.
    IPv6Address neighborMainAddr;
    /// Neighbor Type and Link Type at the four less significative digits.
    enum Status {
        STATUS_NOT_SYM = 0, // "not symmetric"
        STATUS_SYM = 1, // "symmetric"
    } status;
    /// A value between 0 and 7 specifying the node's willingness to carry traffic on behalf of other nodes.
    uint8_t willingness;
};

static inline bool operator == (const NeighborTuple &a, const NeighborTuple &b)
{
    return (a.neighborMainAddr == b.neighborMainAddr &&
            a.status == b.status &&
            a.willingness == b.willingness);
}

// static inline std::ostream& operator << (std::ostream &os, const NeighborTuple &tuple)
// {
//     os  << "NeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
//         << ", status=" << (tuple.status == NeighborTuple::STATUS_SYM ? "SYM" : "NOT_SYM")
//         << ", willingness=" << (int) tuple.willingness << ")";
//     return os;
// }

/// A 2-hop Tuple.
struct TwoHopNeighborTuple
{
    /// Main address of a neighbor.
    IPv6Address neighborMainAddr;
    /// Main address of a 2-hop neighbor with a symmetric link to nb_main_addr.
    IPv6Address twoHopNeighborAddr;
};

// static inline std::ostream& operator << (std::ostream &os, const TwoHopNeighborTuple &tuple)
// {
//     os  << "TwoHopNeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
//         << ", twoHopNeighborAddr=" << tuple.twoHopNeighborAddr
//         << ")";
//     return os;
// }

static inline bool operator == (const TwoHopNeighborTuple &a, const TwoHopNeighborTuple &b)
{
    return (a.neighborMainAddr == b.neighborMainAddr &&
            a.twoHopNeighborAddr == b.twoHopNeighborAddr);
}

/// A Topology Tuple
struct TopologyTuple
{
    /// Main address of the destination.
    IPv6Address destAddr;
    /// Main address of a node which is a neighbor of the destination.
    IPv6Address lastAddr;
    /// Sequence number.
    uint16_t sequenceNumber;
};

static inline bool operator == (const TopologyTuple &a, const TopologyTuple &b)
{
    return (a.destAddr == b.destAddr &&
            a.lastAddr == b.lastAddr &&
            a.sequenceNumber == b.sequenceNumber);
}

// static inline std::ostream& operator << (std::ostream &os, const TopologyTuple &tuple)
// {
//     os  << "TopologyTuple(destAddr=" << tuple.destAddr
//         << ", lastAddr=" << tuple.lastAddr
//         << ", sequenceNumber=" << (int) tuple.sequenceNumber
//         << ")";
//     return os;
// }

/// An Interface Association Tuple.
struct InterfaceAssociationTuple
{
    /// Interface address of a node.
    IPv6Address ifaceAddr;
    /// Main address of the node.
    IPv6Address mainAddr;
};

static inline bool  operator == (const InterfaceAssociationTuple &a, const InterfaceAssociationTuple &b)
{
    return (a.ifaceAddr == b.ifaceAddr && a.mainAddr == b.mainAddr);
}

// static inline std::ostream& operator << (std::ostream &os, const InterfaceAssociationTuple &tuple)
// {
//     os  << "InterfaceAssociationTuple(ifaceAddr=" << tuple.ifaceAddr
//         << ", mainAddr=" << tuple.mainAddr << ")";
//     return os;
// }

// typedef std::set<IPv6Address>                       MprSet;                     ///< MPR Set type.
// typedef std::vector<MprSelectorTuple>               MprSelectorSet;             ///< MPR Selector Set type.
// typedef std::vector<LinkTuple>                      LinkSet;                    ///< Link Set type.
// typedef std::vector<NeighborTuple>                  NeighborSet;                ///< Neighbor Set type.
// typedef std::vector<TwoHopNeighborTuple>            TwoHopNeighborSet;          ///< 2-hop Neighbor Set type.
// typedef std::vector<TopologyTuple>                  TopologySet;                ///< Topology Set type.
// typedef std::vector<InterfaceAssociationTuple>      InterfaceAssociationSet;    ///< Interface Association Set type.


#endif  /* TYPE_H */
