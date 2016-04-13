#include "Headers/OLSRState.h"

OLSRState::OLSRState() {}

/**
*   Find MPR
*/
bool OLSRState::findMprAddress (const MACAddress  &address) {
    std::set<MACAddress>::iterator it = mMprSet.find(address);
    return (it != mMprSet.end());

}

void OLSRState::setMprSet (std::set<MACAddress> mprSet) {
    mMprSet = mprSet;
}

/**
* MPR selector
*/
MprSelectorTuple* OLSRState::findMprSelectorTuple (MACAddress &mainAddr) {
    for (std::vector<MprSelectorTuple>::iterator it = mMprSelectorSet.begin();
            it != mMprSelectorSet.end(); it++) {
        if (it->mainAddr == mainAddr)
            return &(*it);
    }
    return NULL;
}

void OLSRState::cleanMprSelectorTuple (MprSelectorTuple &tuple) {
    for (std::vector<MprSelectorTuple>::iterator it = mMprSelectorSet.begin();
            it != mMprSelectorSet.end(); it++) {
        if (*it == tuple) {
            mMprSelectorSet.erase(it);
            break;
        }
    }
}

void OLSRState::cleanMprSelectorTuples (const MACAddress &mainAddr)
{
    for (std::vector<MprSelectorTuple>::iterator it = mMprSelectorSet.begin();
            it != mMprSelectorSet.end();) {
        if (it->mainAddr == mainAddr) {
            it = mMprSelectorSet.erase(it);
        } else {
            it++;
        }
    }
}

void OLSRState::insertMprSelectorTuple (MprSelectorTuple const &tuple)
{
    mMprSelectorSet.push_back (tuple);
}

std::string OLSRState::printMprSelectorSet() const
{
    std::stringstream os;
    os << "[";
    for (std::vector<MprSelectorTuple>::const_iterator iter = mMprSelectorSet.begin();
            iter != mMprSelectorSet.end(); iter++) {
        std::vector<MprSelectorTuple>::const_iterator next = iter;
        next++;
        os << iter->mainAddr;
        if (next != mMprSelectorSet.end())
            os << ", ";
    }
    os << "]";
    return os.str();
}

/**
*   Neighbor Set Manipulation
*/
NeighborTuple* OLSRState::findNeighborTuple (const MACAddress &mainAddr)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr)
            return &(*it);
    }
    return NULL;
}

const NeighborTuple* OLSRState::findSymNeighborTuple (const MACAddress &mainAddr) const
{
    for (std::vector<NeighborTuple>::const_iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr && it->status == NeighborTuple::STATUS_SYM)
            return &(*it);
    }
    return NULL;
}

NeighborTuple* OLSRState::findNeighborTuple (const MACAddress &mainAddr, uint8_t willingness)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr && it->willingness == willingness)
            return &(*it);
    }
    return NULL;
}

void OLSRState::cleanNeighborTuple (const NeighborTuple &tuple)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (*it == tuple) {
            mNeighborSet.erase(it);
            break;
        }
    }
}

void OLSRState::cleanNeighborTuple (const MACAddress &mainAddr)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr) {
            it = mNeighborSet.erase(it);
            break;
        }
    }
}

void OLSRState::insertNeighborTuple (const NeighborTuple &tuple)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == tuple.neighborMainAddr) {
            // Update it
            *it = tuple;
            return;
        }
    }
    mNeighborSet.push_back (tuple);
}



/**
* Neighbor 2 Hop Set Manipulation
*/
TwoHopNeighborTuple* OLSRState::findTwoHopNeighborTuple (const MACAddress &neighborMainAddr, const MACAddress &twoHopNeighborAddr)
{
    for (std::vector<TwoHopNeighborTuple>::iterator it = mTwoHopNeighborSet.begin(); it != mTwoHopNeighborSet.end(); it++) {
        if (it->neighborMainAddr == neighborMainAddr && it->twoHopNeighborAddr == twoHopNeighborAddr) {
            return &(*it);
        }
    }
    return NULL;
}

void OLSRState::cleanTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
    for (std::vector<TwoHopNeighborTuple>::iterator it = mTwoHopNeighborSet.begin(); it != mTwoHopNeighborSet.end(); it++) {
        if (*it == tuple) {
            mTwoHopNeighborSet.erase(it);
            break;
        }
    }
}

void OLSRState::cleanTwoHopNeighborTuples (const MACAddress &neighborMainAddr, const MACAddress &twoHopNeighborAddr)
{
    for (std::vector<TwoHopNeighborTuple>::iterator it = mTwoHopNeighborSet.begin(); it != mTwoHopNeighborSet.end();) {
        if (it->neighborMainAddr == neighborMainAddr
                && it->twoHopNeighborAddr == twoHopNeighborAddr) {
            it = mTwoHopNeighborSet.erase(it);
        } else {
            it++;
        }
    }
}

void OLSRState::cleanTwoHopNeighborTuples (const MACAddress &neighborMainAddr)
{
    for (std::vector<TwoHopNeighborTuple>::iterator it = mTwoHopNeighborSet.begin(); it != mTwoHopNeighborSet.end();) {
        if (it->neighborMainAddr == neighborMainAddr) {
            it = mTwoHopNeighborSet.erase(it);
        } else {
            it++;
        }
    }
}

void OLSRState::insertTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
    mTwoHopNeighborSet.push_back(tuple);
}


/**
* Link Set Manipulation
*/
LinkTuple* OLSRState::findLinkTuple (const MACAddress & ifaceAddr)
{
    for (std::vector<LinkTuple> ::iterator it = mLinkSet.begin(); it != mLinkSet.end(); it++) {
        if (it->neighborIfaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

LinkTuple* OLSRState::findSymLinkTuple (const MACAddress &ifaceAddr)
{
    for (std::vector<LinkTuple> ::iterator it = mLinkSet.begin(); it != mLinkSet.end(); it++) {
        if (it->neighborIfaceAddr == ifaceAddr) {
            return &(*it);
        }
    }
    return NULL;
}

void OLSRState::cleanLinkTuple (const LinkTuple &tuple)
{
    for (std::vector<LinkTuple> ::iterator it = mLinkSet.begin(); it != mLinkSet.end(); it++) {
        if (*it == tuple) {
            mLinkSet.erase(it);
            break;
        }
    }
}

LinkTuple& OLSRState::insertLinkTuple (const LinkTuple &tuple)
{
    mLinkSet.push_back(tuple);
    return mLinkSet.back();
}

/**
* Topology Set Manipulation
*/
TopologyTuple* OLSRState::findTopologyTuple (const MACAddress &destAddr, const MACAddress &lastAddr)
{
    for (std::vector<TopologyTuple>::iterator it = mTopologySet.begin(); it != mTopologySet.end(); it++) {
        if (it->destAddr == destAddr && it->lastAddr == lastAddr)
            return &(*it);
    }
    return NULL;
}

TopologyTuple* OLSRState::findNewerTopologyTuple (const MACAddress & lastAddr, uint16_t ansn)
{
    for (std::vector<TopologyTuple>::iterator it = mTopologySet.begin(); it != mTopologySet.end(); it++) {
        if (it->lastAddr == lastAddr && it->sequenceNumber > ansn)
            return &(*it);
    }
    return NULL;
}

void OLSRState::cleanTopologyTuple(const TopologyTuple &tuple)
{
    for (std::vector<TopologyTuple>::iterator it = mTopologySet.begin(); it != mTopologySet.end(); it++)
    {
        if (*it == tuple) {
            mTopologySet.erase(it);
            break;
        }
    }
}

void OLSRState::cleanOlderTopologyTuples (const MACAddress &lastAddr, uint16_t ansn)
{
    for (std::vector<TopologyTuple>::iterator it = mTopologySet.begin(); it != mTopologySet.end();) {
        if (it->lastAddr == lastAddr && it->sequenceNumber < ansn) {
            it = mTopologySet.erase(it);
        } else {
            it++;
        }
    }
}

void OLSRState::insertTopologyTuple (TopologyTuple const &tuple)
{
    mTopologySet.push_back (tuple);
}

/**
* Interface Association Set Manipulation
*/
InterfaceAssociationTuple* OLSRState::findInterfaceAssociationTuple (const MACAddress &ifaceAddr)
{
    for (std::vector<InterfaceAssociationTuple>::iterator it = mInterfaceAssociationSet.begin(); it != mInterfaceAssociationSet.end(); it++) {
        if (it->ifaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

const InterfaceAssociationTuple* OLSRState::findInterfaceAssociationTuple (const MACAddress &ifaceAddr) const
{
    for (std::vector<InterfaceAssociationTuple>::const_iterator it = mInterfaceAssociationSet.begin(); it != mInterfaceAssociationSet.end(); it++) {
        if (it->ifaceAddr == ifaceAddr)
            return &(*it);
    }
    return NULL;
}

void OLSRState::cleanInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple)
{
    for (std::vector<InterfaceAssociationTuple>::iterator it = mInterfaceAssociationSet.begin(); it != mInterfaceAssociationSet.end(); it++) {
        if (*it == tuple) {
            mInterfaceAssociationSet.erase(it);
            break;
        }
    }
}

void OLSRState::insertInterfaceAssociationTuple (const InterfaceAssociationTuple &tuple)
{
    mInterfaceAssociationSet.push_back(tuple);
}

std::vector<MACAddress> OLSRState::findNeighborInterfaces (const MACAddress &neighborMainAddr) const
{
    std::vector<MACAddress> retval;
    for (std::vector<InterfaceAssociationTuple>::const_iterator it = mInterfaceAssociationSet.begin(); it != mInterfaceAssociationSet.end(); it++) {
        if (it->mainAddr == neighborMainAddr)
            retval.push_back (it->ifaceAddr);
    }
    return retval;
}
