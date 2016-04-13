#ifndef OLSR_STATE_H
#define OLSR_STATE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include "Resources/Types.h"
#include "MACAddress.h"

//using namespace std;

/*
 * State inspired from NS3 OLSR Routing
 */
class OLSRState
{
private:

public:
    OLSRState();
    bool mIsMPR;

    std::set<MACAddress>                       mMprSet;                     ///< MPR Set type.
    std::vector<MprSelectorTuple>               mMprSelectorSet;             ///< MPR Selector Set type.
    std::vector<LinkTuple>                      mLinkSet;                    ///< Link Set type.
    std::vector<NeighborTuple>                  mNeighborSet;                ///< Neighbor Set type.
    std::vector<TwoHopNeighborTuple>            mTwoHopNeighborSet;          ///< 2-hop Neighbor Set type.
    std::vector<TopologyTuple>                  mTopologySet;                ///< Topology Set type.
    std::vector<InterfaceAssociationTuple>      mInterfaceAssociationSet;    ///< Interface Association Set type.

    // getter:
    std::vector<MprSelectorTuple>& getMprSelectors ()             { return mMprSelectorSet;            }
    std::vector<LinkTuple>& getLinks ()                           { return mLinkSet;                   }
    std::vector<NeighborTuple>& getNeighbors ()                   { return mNeighborSet;               }
    std::vector<TwoHopNeighborTuple>& getTwoHopNeighbors ()       { return mTwoHopNeighborSet;         }
    std::vector<TopologyTuple>& getTopologySet ()                 { return mTopologySet;               }
    std::vector<InterfaceAssociationTuple>& getIfaceAssocSet ()   { return mInterfaceAssociationSet;   }

    // MPR
    bool findMprAddress (const MACAddress  &address);
    void setMprSet (std::set<MACAddress> mprSet);

    // MPR selector
    MprSelectorTuple* findMprSelectorTuple (MACAddress &mainAddr);
    void cleanMprSelectorTuple (MprSelectorTuple &tuple);
    void cleanMprSelectorTuples (const MACAddress &mainAddr);
    void insertMprSelectorTuple (const MprSelectorTuple &tuple);
    std::string printMprSelectorSet () const;

    // Link
    LinkTuple* findLinkTuple (const MACAddress &ifaceAddr);
    LinkTuple* findSymLinkTuple (const MACAddress &ifaceAddr);
    void cleanLinkTuple (const LinkTuple &tuple);
    LinkTuple& insertLinkTuple (const LinkTuple &tuple);

    // Neighbor
    NeighborTuple* findNeighborTuple (const MACAddress &mainAddr);
    const NeighborTuple* findSymNeighborTuple (const MACAddress &mainAddr) const;
    NeighborTuple* findNeighborTuple (const MACAddress &mainAddr, uint8_t willingness);
    void cleanNeighborTuple (const NeighborTuple &neighborTuple);
    void cleanNeighborTuple (const MACAddress &mainAddr);
    void insertNeighborTuple (const NeighborTuple &tuple);

    // Two-hop neighbor
    TwoHopNeighborTuple* findTwoHopNeighborTuple (const MACAddress &neighbor, const MACAddress &twoHopNeighbor);
    void cleanTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);
    void cleanTwoHopNeighborTuples (const MACAddress &neighbor);
    void cleanTwoHopNeighborTuples (const MACAddress &neighbor, const MACAddress &twoHopNeighbor);
    void insertTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);

    // Topology
    TopologyTuple* findTopologyTuple (const MACAddress &destAddr, const MACAddress &lastAddr);
    TopologyTuple* findNewerTopologyTuple (const MACAddress &lastAddr, uint16_t ansn);
    void cleanTopologyTuple (const TopologyTuple &tuple);
    void cleanOlderTopologyTuples (const MACAddress &lastAddr, uint16_t ansn);
    void insertTopologyTuple (const TopologyTuple &tuple);

    // Interface association
    InterfaceAssociationTuple* findInterfaceAssociationTuple (const MACAddress &ifaceAddr);
    const InterfaceAssociationTuple* findInterfaceAssociationTuple (const MACAddress &ifaceAddr) const;
    void cleanInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);
    void insertInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);

    // Returns a std::vector of all interfaces of a given neighbor, with the
    // exception of the "main" one.
    std::vector<MACAddress> findNeighborInterfaces (const MACAddress &neighborMainAddr) const;
};

#endif // OLSR_STATE_H
