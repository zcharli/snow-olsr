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
            // std::cout << "Erasing mpr selector" << std::endl;
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
            // std::cout << "Erasing mpr selector" << std::endl;
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
            // std::cout << "Erased neigbor tuple" << std::endl;
            mNeighborSet.erase(it);
            break;
        }
    }
}

void OLSRState::cleanNeighborTuple (const MACAddress &mainAddr)
{
    for (std::vector<NeighborTuple>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); it++) {
        if (it->neighborMainAddr == mainAddr) {
            // std::cout << "Erased neigbor tuple" << std::endl;
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
            // std::cout << "Erased two hop neigbor tuple" << std::endl;
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
            //std::cout << "Erased two hop neigbor tuple" << std::endl;
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
            //std::cout << "Erased two hop neigbor tuple" << std::endl;
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

LinkTuple* OLSRState::findSymLinkTuple (const MACAddress &ifaceAddr, pt::ptime now)
{
    for (std::vector<LinkTuple> ::iterator it = mLinkSet.begin(); it != mLinkSet.end(); it++) {
        if (it->neighborIfaceAddr == ifaceAddr) {
            if (it->neighborIfaceAddr == ifaceAddr) {
               // if (it->symTime > now)
                    return &(*it);
                // else
                //     break;
            }
        }
    }
    return NULL;
}

void OLSRState::cleanLinkTuple (const LinkTuple &tuple)
{
    for (std::vector<LinkTuple> ::iterator it = mLinkSet.begin(); it != mLinkSet.end(); it++) {
        if (*it == tuple) {
            //std::cout << "Erased link tuple" << std::endl;
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
            //std::cout << "Erased link tuple" << std::endl;
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
 * Duplicate Set Manipulation
 */

DuplicateTuple* OLSRState::findDuplicateTuple (MACAddress const &addr, uint16_t sequenceNumber) {
    for (std::vector<DuplicateTuple>::iterator it = mDuplicateSet.begin ();
            it != mDuplicateSet.end(); it++) {
        if (it->address == addr && it->sequenceNumber == sequenceNumber)
            return &(*it);
    }
    return NULL;
}

void OLSRState::eraseDuplicateTuple (const DuplicateTuple &tuple) {
    for (std::vector<DuplicateTuple>::iterator it = mDuplicateSet.begin ();
            it != mDuplicateSet.end (); it++) {
        if (*it == tuple)
        {
            mDuplicateSet.erase (it);
            break;
        }
    }
}

void OLSRState::insertDuplicateTuple (DuplicateTuple const &tuple) {
    mDuplicateSet.push_back (tuple);
}
