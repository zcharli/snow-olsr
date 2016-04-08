#include "Headers/RoutingTable.h"

RoutingTable& RoutingTable::getInstance() {
    static RoutingTable instance;
    return instance;
}

/********** MPR Selector Set Manipulation **********/
MprSelectorTuple* RoutingTable::FindMprSelectorTuple (const Ipv4Address &mainAddr)
{
    for (MprSelectorSet::iterator it = mprSelectorSet.begin(); it != mprSelectorSet.end(); it++) {
        if (it->mainAddr == mainAddr)
            return &(*it);
    }
    return NULL;
}

void RoutingTable::CleanMprSelectorTuple (const MprSelectorTuple &tuple)
{
    for (MprSelectorSet::iterator it = mprSelectorSet.begin(); it != mprSelectorSet.end(); it++) {
        if (*it == tuple) {
            mprSelectorSet.erase(it);
            break;
        }
    }
}

void RoutingTable::CleanMprSelectorTuples (const Ipv4Address &mainAddr)
{
    for (MprSelectorSet::iterator it = mprSelectorSet.begin(); it != mprSelectorSet.end();) {
        if (it->mainAddr == mainAddr) {
            it = mprSelectorSet.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingTable::InsertMprSelectorTuple (MprSelectorTuple const &tuple)
{
    mprSelectorSet.push_back (tuple);
}

std::string RoutingTable::PrintMprSelectorSet() const
{
    std::ostringstream os;
    os << "[";
    for (MprSelectorSet::const_iterator iter = mprSelectorSet.begin(); iter != mprSelectorSet.end(); iter++) {
        MprSelectorSet::const_iterator next = iter;
        next++;
        os << iter->mainAddr;
        if (next != mprSelectorSet.end())
            os << ", ";
    }
    os << "]";
    return os.str();
}

/********** Neighbor Set Manipulation **********/
NeighborTuple* RoutingTable::FindNeighborTuple (const Ipv4Address &mainAddr)
{
    for (NeighborSet::iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr)
            return &(*it);
    }
    return NULL;
}

const NeighborTuple* RoutingTable::FindSymNeighborTuple (const Ipv4Address &mainAddr) const
{
    for (NeighborSet::const_iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr && it->status == NeighborTuple::STATUS_SYM)
            return &(*it);
    }
    return NULL;
}

NeighborTuple* RoutingTable::FindNeighborTuple (const Ipv4Address &mainAddr, uint8_t willingness)
{
    for (NeighborSet::iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr && it->willingness == willingness)
            return &(*it);
    }
    return NULL;
}

void RoutingTable::CleanNeighborTuple (const NeighborTuple &tuple)
{
    for (NeighborSet::iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (*it == tuple) {
            neighborSet.erase(it);
            break;
        }
    }
}

void RoutingTable::CleanNeighborTuple (const Ipv4Address &mainAddr)
{
    for (NeighborSet::iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr) {
            it = neighborSet.erase(it);
            break;
        }
    }
}

void RoutingTable::InsertNeighborTuple (const NeighborTuple &tuple)
{
    for (NeighborSet::iterator it = neighborSet.begin(); it != neighborSet.end(); it++) {
        if (it->neighborMainAddr == tuple.neighborMainAddr) {
            // Update it
            *it = tuple;
            return;
        }
    }
    neighborSet.push_back (tuple);
}

/********** Neighbor 2 Hop Set Manipulation **********/
TwoHopNeighborTuple* RoutingTable::FindTwoHopNeighborTuple (const Ipv4Address &neighborMainAddr, const Ipv4Address &twoHopNeighborAddr)
{
    for (TwoHopNeighborSet::iterator it = twoHopNeighborSet.begin(); it != twoHopNeighborSet.end(); it++) {
        if (it->neighborMainAddr == neighborMainAddr && it->twoHopNeighborAddr == twoHopNeighborAddr) {
            return &(*it);
        }
    }
    return NULL;
}

void RoutingTable::CleanTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
    for (TwoHopNeighborSet::iterator it = twoHopNeighborSet.begin(); it != twoHopNeighborSet.end(); it++) {
        if (*it == tuple) {
            twoHopNeighborSet.erase(it);
            break;
        }
    }
}

void RoutingTable::CleanTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr, const Ipv4Address &twoHopNeighborAddr)
{
    for (TwoHopNeighborSet::iterator it = twoHopNeighborSet.begin(); it != twoHopNeighborSet.end();) {
        if (it->neighborMainAddr == neighborMainAddr
                && it->twoHopNeighborAddr == twoHopNeighborAddr) {
            it = twoHopNeighborSet.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingTable::CleanTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr)
{
    for (TwoHopNeighborSet::iterator it = twoHopNeighborSet.begin(); it != twoHopNeighborSet.end();) {
        if (it->neighborMainAddr == neighborMainAddr) {
            it = twoHopNeighborSet.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingTable::InsertTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
    twoHopNeighborSet.push_back(tuple);
}

/********** MPR Set Manipulation **********/
bool RoutingTable::FindMprAddress (const Ipv4Address &addr)
{
    MprSet::iterator it = mprSet.find(addr);
    return (it != mprSet.end());
}

void RoutingTable::SetMprSet (MprSet mprSet)
{
    mprSet = mprSet;
}

/********** Link Set Manipulation **********/
LinkTuple* RoutingTable::FindLinkTuple (const Ipv4Address & ifaceAddr)
{
    for (LinkSet::iterator it = linkSet.begin(); it != linkSet.end(); it++) {
        if (it->neighborIfaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

LinkTuple* RoutingTable::FindSymLinkTuple (const Ipv4Address &ifaceAddr)
{
    for (LinkSet::iterator it = linkSet.begin(); it != linkSet.end(); it++) {
        if (it->neighborIfaceAddr == ifaceAddr) {
            return &(*it);
        }
    }
    return NULL;
}

void RoutingTable::CleanLinkTuple (const LinkTuple &tuple)
{
    for (LinkSet::iterator it = linkSet.begin(); it != linkSet.end(); it++) {
        if (*it == tuple) {
            linkSet.erase(it);
            break;
        }
    }
}

LinkTuple& RoutingTable::InsertLinkTuple (const LinkTuple &tuple)
{
    linkSet.push_back(tuple);
    return linkSet.back();
}

/********** Topology Set Manipulation **********/
TopologyTuple* RoutingTable::FindTopologyTuple (const Ipv4Address &destAddr, const Ipv4Address &lastAddr)
{
    for (TopologySet::iterator it = topologySet.begin(); it != topologySet.end(); it++) {
        if (it->destAddr == destAddr && it->lastAddr == lastAddr)
            return &(*it);
    }
    return NULL;
}

TopologyTuple* RoutingTable::FindNewerTopologyTuple (const Ipv4Address & lastAddr, uint16_t ansn)
{
    for (TopologySet::iterator it = topologySet.begin(); it != topologySet.end(); it++) {
        if (it->lastAddr == lastAddr && it->sequenceNumber > ansn)
            return &(*it);
    }
    return NULL;
}

void RoutingTable::CleanTopologyTuple(const TopologyTuple &tuple)
{
    for (TopologySet::iterator it = topologySet.begin(); it != topologySet.end(); it++)
    {
        if (*it == tuple) {
            topologySet.erase(it);
            break;
        }
    }
}

void RoutingTable::CleanOlderTopologyTuples (const Ipv4Address &lastAddr, uint16_t ansn)
{
    for (TopologySet::iterator it = topologySet.begin(); it != topologySet.end();) {
        if (it->lastAddr == lastAddr && it->sequenceNumber < ansn) {
            it = topologySet.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingTable::InsertTopologyTuple (TopologyTuple const &tuple)
{
    topologySet.push_back (tuple);
}

/********** Interface Association Set Manipulation **********/
InterfaceAssociationTuple* RoutingTable::FindInterfaceAssociationTuple (const Ipv4Address &ifaceAddr)
{
    for (InterfaceAssociationSet::iterator it = interfaceAssociationSet.begin(); it != interfaceAssociationSet.end(); it++) {
        if (it->ifaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

const InterfaceAssociationTuple* RoutingTable::FindInterfaceAssociationTuple (const Ipv4Address &ifaceAddr) const
{
    for (InterfaceAssociationSet::const_iterator it = interfaceAssociationSet.begin(); it != interfaceAssociationSet.end(); it++) {
        if (it->ifaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

void RoutingTable::CleanInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple)
{
    for (InterfaceAssociationSet::iterator it = interfaceAssociationSet.begin(); it != interfaceAssociationSet.end(); it++) {
        if (*it == tuple) {
            interfaceAssociationSet.erase(it);
            break;
        }
    }
}

void RoutingTable::InsertInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple)
{
    interfaceAssociationSet.push_back(tuple);
}

std::vector<Ipv4Address> RoutingTable::FindNeighborInterfaces (const Ipv4Address &neighborMainAddr) const
{
    std::vector<Ipv4Address> retval;
    for (InterfaceAssociationSet::const_iterator it = interfaceAssociationSet.begin(); it != interfaceAssociationSet.end(); it++) {
        if (it->mainAddr == neighborMainAddr)
            retval.push_back (it->ifaceAddr);
    }
    return retval;
}
