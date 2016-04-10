#ifndef OLSR_STATE_H
#define OLSR_STATE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include "Resources/Types.h"
#include "IPv6Address.h"

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

    set<IPv6Address>                       mMprSet;                     ///< MPR Set type.
    vector<MprSelectorTuple>               mMprSelectorSet;             ///< MPR Selector Set type.
    vector<LinkTuple>                      mLinkSet;                    ///< Link Set type.
    vector<NeighborTuple>                  mNeighborSet;                ///< Neighbor Set type.
    vector<TwoHopNeighborTuple>            mTwoHopNeighborSet;          ///< 2-hop Neighbor Set type.
    vector<TopologyTuple>                  mTopologySet;                ///< Topology Set type.
    vector<InterfaceAssociationTuple>      mInterfaceAssociationSet;    ///< Interface Association Set type.
    
    //RoutingTable(RoutingTable&);
    void makeMPR();

    //void operator=(RoutingTable&);
    

    // getter:
    vector<MprSelectorTuple>& getMprSelectors ()             { return mMprSelectorSet;            }
    vector<LinkTuple>& getLinks ()                           { return mLinkSet;                   }
    vector<NeighborTuple>& getNeighbors ()                   { return mNeighborSet;               }
    vector<TwoHopNeighborTuple>& getTwoHopNeighbors ()       { return mTwoHopNeighborSet;         }
    vector<TopologyTuple>& getTopologySet ()                 { return mTopologySet;               }
    vector<InterfaceAssociationTuple>& getIfaceAssocSet ()   { return mInterfaceAssociationSet;   }

    // MPR
    bool findMprAddress (const IPv6Address  &address);
    void setMprSet (set<IPv6Address> mprSet);

    // MPR selector
    MprSelectorTuple* findMprSelectorTuple (IPv6Address &mainAddr);
    void cleanMprSelectorTuple (MprSelectorTuple &tuple);
    void cleanMprSelectorTuples (const IPv6Address &mainAddr);
    void insertMprSelectorTuple (const MprSelectorTuple &tuple);
    string printMprSelectorSet () const;

    // Link
    LinkTuple* findLinkTuple (const IPv6Address &ifaceAddr);
    LinkTuple* findSymLinkTuple (const IPv6Address &ifaceAddr);
    void cleanLinkTuple (const LinkTuple &tuple);
    LinkTuple& insertLinkTuple (const LinkTuple &tuple);

    // Neighbor
    NeighborTuple* findNeighborTuple (const IPv6Address &mainAddr);
    const NeighborTuple* findSymNeighborTuple (const IPv6Address &mainAddr) const;
    NeighborTuple* findNeighborTuple (const IPv6Address &mainAddr, uint8_t willingness);
    void cleanNeighborTuple (const NeighborTuple &neighborTuple);
    void cleanNeighborTuple (const IPv6Address &mainAddr);
    void insertNeighborTuple (const NeighborTuple &tuple);

    // Two-hop neighbor
    TwoHopNeighborTuple* findTwoHopNeighborTuple (const IPv6Address &neighbor, const IPv6Address &twoHopNeighbor);
    void cleanTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);
    void cleanTwoHopNeighborTuples (const IPv6Address &neighbor);
    void cleanTwoHopNeighborTuples (const IPv6Address &neighbor, const IPv6Address &twoHopNeighbor);
    void insertTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);

    // Topology
    TopologyTuple* findTopologyTuple (const IPv6Address &destAddr, const IPv6Address &lastAddr);
    TopologyTuple* findNewerTopologyTuple (const IPv6Address &lastAddr, uint16_t ansn);
    void cleanTopologyTuple (const TopologyTuple &tuple);
    void cleanOlderTopologyTuples (const IPv6Address &lastAddr, uint16_t ansn);
    void insertTopologyTuple (const TopologyTuple &tuple);

    // Interface association
    InterfaceAssociationTuple* findInterfaceAssociationTuple (const IPv6Address &ifaceAddr);
    const InterfaceAssociationTuple* findInterfaceAssociationTuple (const IPv6Address &ifaceAddr) const;
    void cleanInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);
    void insertInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple);

    // Returns a vector of all interfaces of a given neighbor, with the
    // exception of the "main" one.
    vector<IPv6Address> findNeighborInterfaces (const IPv6Address &neighborMainAddr) const;
};

#endif // OLSR_STATE_H
