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
#include "../MACAddress.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
/*
    Introduction of Type.h for OLSR

    Type Name                   Description                         Data Type
    MprSet:                     MPR Set type                        std::set<MACAddress>
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

//#define MACAddress char

/// An MPR-Selector Tuple.
struct MprSelectorTuple
{
    /// Main address of a node which have selected this node as a MPR.
    MACAddress mainAddr;
    /// Time at which this tuple expires and must be removed.
    boost::posix_time::ptime expirationTime;
};

static inline bool operator == (const MprSelectorTuple &a, const MprSelectorTuple &b)
{
    return (a.mainAddr == b.mainAddr);
}

/// A Link Tuple.
struct LinkTuple
{
    /// Interface address of the local node.
    MACAddress localIfaceAddr;
    /// Interface address of the neighbor node.
    MACAddress neighborIfaceAddr;
    /// The link is considered bidirectional until this time.
    boost::posix_time::ptime symTime;
    /// The link is considered unidirectional until this time.
    boost::posix_time::ptime asymTime;
    /// Time at which this tuple expires and must be removed.
    boost::posix_time::ptime expirationTime;
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
    MACAddress neighborMainAddr;
    /// Neighbor Type and Link Type at the four less significative digits.
    enum Status {
        STATUS_NOT_SYM = 0, // "not symmetric"
        STATUS_SYM = 1, // "symmetric"
    } status;
    /// A value between 0 and 7 specifying the node's willingness to carry traffic on behalf of other nodes.
    uint8_t willingness;
    boost::posix_time::ptime expirationTime;
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
    MACAddress neighborMainAddr;
    /// Main address of a 2-hop neighbor with a symmetric link to nb_main_addr.
    MACAddress twoHopNeighborAddr;
    boost::posix_time::ptime expirationTime;
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
    MACAddress destAddr;
    /// Main address of a node which is a neighbor of the destination.
    MACAddress lastAddr;
    /// Sequence number.
    uint16_t sequenceNumber;
    boost::posix_time::ptime expirationTime;
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
    MACAddress ifaceAddr;
    /// Main address of the node.
    MACAddress mainAddr;
};

static inline bool  operator == (const InterfaceAssociationTuple &a, const InterfaceAssociationTuple &b)
{
    return (a.ifaceAddr == b.ifaceAddr && a.mainAddr == b.mainAddr);
}

/// A Duplicate Tuple
struct DuplicateTuple
{
  /// Originator address of the message.
  MACAddress address;
  /// Message sequence number.
  uint16_t sequenceNumber;
  /// Indicates whether the message has been retransmitted or not.
  bool retransmitted;
  /// List of interfaces which the message has been received on.
  std::vector<MACAddress> ifaceList;
  /// Time at which this tuple expires and must be removed.
  boost::posix_time::ptime expirationTime;
};

static inline bool
operator == (const DuplicateTuple &a, const DuplicateTuple &b)
{
  return (a.address == b.address
          && a.sequenceNumber == b.sequenceNumber);
}

// static inline std::ostream& operator << (std::ostream &os, const InterfaceAssociationTuple &tuple)
// {
//     os  << "InterfaceAssociationTuple(ifaceAddr=" << tuple.ifaceAddr
//         << ", mainAddr=" << tuple.mainAddr << ")";
//     return os;
// }

// typedef std::set<MACAddress>                       MprSet;                     ///< MPR Set type.
// typedef std::vector<MprSelectorTuple>               MprSelectorSet;             ///< MPR Selector Set type.
// typedef std::vector<LinkTuple>                      LinkSet;                    ///< Link Set type.
// typedef std::vector<NeighborTuple>                  NeighborSet;                ///< Neighbor Set type.
// typedef std::vector<TwoHopNeighborTuple>            TwoHopNeighborSet;          ///< 2-hop Neighbor Set type.
// typedef std::vector<TopologyTuple>                  TopologySet;                ///< Topology Set type.
// typedef std::vector<InterfaceAssociationTuple>      InterfaceAssociationSet;    ///< Interface Association Set type.


#endif  /* TYPE_H */
