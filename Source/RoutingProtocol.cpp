#include "Headers/RoutingProtocol.h"

void RoutingProtocol::updateState(std::shared_ptr<OLSRMessage> message) {
    // Here we want to hand
    for (std::vector<Message>::iterator it = message->messages.begin();
            it != message->messages.end(); it++) {
        switch (it->getType()) {
        case M_HELLO_MESSAGE :
            // Dereferenced HelloMsg from the address of the dereference iterator msg
            handleHelloMessage(*((HelloMessage*) & (*it)), message->mSenderHWAddr, it->getVTime());
            break;
        case M_TC_MESSAGE:
            // Handle a TC
            handleTCMessage(*((TCMessage*) & (*it)), message->mSenderHWAddr);
            break;
        case M_MID_MESSAGE:
            // Not implemented
            break;
        case M_HNA_MESSAGE:
            // Not implemented
            break;
        }
    }
}

void RoutingProtocol::handleHelloMessage(HelloMessage& message, const IPv6Address& senderHWAddr, unsigned char vtime) {
    mMtxState.lock();
    LinkTuple* vLinkEdge = mState.findLinkTuple(senderHWAddr);
    mMtxState.unlock();
    bool create = false;
    bool update = false;
    pt::ptime now = pt::second_clock::local_time();
    if (vLinkEdge == NULL) {
        PRINTLN(Found a new neighbor from hello msg)
        LinkTuple newNeighbor;
        newNeighbor.neighborIfaceAddr = senderHWAddr;
        newNeighbor.localIfaceAddr = mPersonalAddress;
        newNeighbor.symTime = now - pt::seconds(1); // Provide the expire time as of now (update later)
        newNeighbor.expirationTime = now + pt::seconds(vtime); // temp
        mMtxState.lock();
        vLinkEdge = &mState.insertLinkTuple(newNeighbor);
        mMtxState.unlock();
        create = true;
    } else {
        PRINTLN(Recv a hello from an existing neighbor)
        update = true;
    }
    // Assume asymetric 4.2.1
    vLinkEdge->asymTime = now + pt::seconds(vtime);

    // This step is unessary, but will do anyways 8.1.1 (processing willingness)
    mMtxState.lock();
    NeighborTuple *neighbor = mState.findNeighborTuple(*(message.getOriginatorAddress()));
    mMtxState.unlock();
    if (neighbor != NULL)
    {
        neighbor->willingness = message.getWillingness();
    }

    // Being computing MPR and bunch of other stuff
    for (std::vector<HelloMessage::LinkMessage>::iterator linkNeighbrs = message.mLinkMessages.begin();
            linkNeighbrs != message.mLinkMessages.end(); linkNeighbrs++ ) {
        int linkType = linkNeighbrs->linkCode & 0x03; // ^C Terminiation
        int neighborType = (linkNeighbrs->linkCode >> 2) & 0x03;
        std::cout << "Hello msg with Link Type: " << linkType << " and Neighbor Type: " << neighborType << std::endl;
        // Skip invalid link and neightbor codes RFC 6.1.1
        if ((linkType == L_SYM_LINK && neighborType == N_NOT_NEIGH) ||
                (neighborType != N_SYM_NEIGH && neighborType != N_MPR_NEIGH
                 && neighborType != N_NOT_NEIGH))
        {
            PRINTLN(Found invalid link type and skipping)
            continue;
        }

        // Process the advertised neighbors
        for (std::vector<IPv6Address>::iterator it = linkNeighbrs->neighborIfAddr.begin();
                it != linkNeighbrs->neighborIfAddr.end(); it++) {
            if (*it == mPersonalAddress) {
                if (!update) {
                    if (linkType == L_LOST_LINK) {
                        // Happens if we have gotten a msg from a neighbor that lost us.
                        vLinkEdge->symTime = now - pt::seconds(1);
                        PRINTLN(An advertised neighbor sent a link lost in hello message)
                        update = true;
                    } else if (linkType == L_ASYM_LINK || linkType == L_SYM_LINK) {
                        // Since we have just sensed the neighbor reached us
                        PRINTLN(A neightbor has sensed us and link between us is bi directional)
                        vLinkEdge->symTime = now + pt::seconds(vtime);
                        vLinkEdge->expirationTime = vLinkEdge->symTime  + pt::seconds(T_NEIGHB_HOLD_TIME);
                        update = true;
                    } else {
                        PRINTLN(Corrupt link type in hello msg)
                    }
                }
            } else {
                PRINTLN(Found a address in hello msg that was not supposed to be for us)
                }
        }
    }
    // Will expire this link soon if asymTime is greater
    vLinkEdge->expirationTime = std::max(vLinkEdge->expirationTime, vLinkEdge->asymTime);
    // Update the changes we made on this edge
    if (update) {
        updateLinkTuple(vLinkEdge, message.getWillingness());
    }

    if (create && vLinkEdge != NULL) {
        // TODO: Schedule a timer to to expire this link
        pt::time_duration expireTimer = vLinkEdge->expirationTime < vLinkEdge->symTime ?
                                        vLinkEdge->symTime - vLinkEdge->expirationTime
                                        : vLinkEdge->symTime - vLinkEdge->expirationTime;
        boost::asio::io_service *mIo = new boost::asio::io_service();
        boost::asio::deadline_timer *vRepeatingTimer = new boost::asio::deadline_timer(*mIo, pt::seconds(expireTimer.total_seconds()));
        vRepeatingTimer->async_wait(boost::bind(&RoutingProtocol::expireLink, this, boost::asio::placeholders::error, vRepeatingTimer, mIo, vLinkEdge->neighborIfaceAddr));
        mIo->run();
    }
}


void RoutingProtocol::handleTCMessage(TCMessage& message, IPv6Address& senderHWAddr) {
    // Double checked with RFC 3626 with TC message processing
    // Should follow the pattern of the handling tc message will be great due to mState is already implemented
    // Either discuss for change or keep in this pattern

    //  1.  If the sender interface (NB: not originator) of this message is not in the symmetric 1-hotp neighborhodd
    //  of this node, the message MUST be discarded.
    mMtxState.lock();
    LinkTuple* linkTuple = mState.findSymLinkTuple(senderHWAddr);
    mMtxState.unlock();
    pt::ptime now = pt::second_clock::local_time();
    if (linkTuple == NULL) return;

    //  2. If there exist some tuple in the topology set where:
    //          T_last_addr == originator address AND
    //          T_seq       >   ANSN,
    //  then further processing of this TC message MUST NOT be performed and the message MUST besilently discard
    //  (case: message received out of order).
    IPv6Address lastAddr =  *(message.getOriginatorAddress());
    uint16_t ansn = message.getAnsn();
    TopologyTuple* vTopologyTuple = mState.findNewerTopologyTuple(lastAddr, ansn);
    if (vTopologyTuple != NULL) return;


    //  3. All tuples in the topology set where:
    //          T_last_addr == originator address AND
    //          T_seq       <   ANSN
    //  MUST be removed from the topology set.
    mState.cleanOlderTopologyTuples(lastAddr, ansn);

    //  4.  For each of the advertised neighbor main address received in the TC message:
    //  Process the advertised neighbors
    for (std::vector<IPv6Address>::const_iterator it = message.mNeighborAddresses.begin(); it != message.mNeighborAddresses.end(); it++)
    {
        //      4.1 If there exist some tuple in the topology set where:
        //                  T_dest_addr == advertised neighbor main address, AND
        //                  T_last_addr == originator address,
        //          then the holding time of that tuple MUST be set to:
        //                  T_time      = current time + validity time.
        //      4.2 Otherwise, a new tuple MUST be recorded in the topology set where:
        //                  T_dest_addr = advertised neighbor main address,
        //                  T_last_addr = originator address,
        //                  T_seq       = ANSN,
        //                  T_time      = current time + validity time
        const IPv6Address &addr = *it;
        vTopologyTuple = mState.findTopologyTuple(senderHWAddr, lastAddr);
        if (vTopologyTuple != NULL) {
            vTopologyTuple->expirationTime = now + pt::seconds(T_NEIGHB_HOLD_TIME);
        } else {
            TopologyTuple topologyTuple;
            topologyTuple.destAddr = addr;
            topologyTuple.lastAddr = lastAddr;
            topologyTuple.sequenceNumber = ansn;
            topologyTuple.expirationTime = vTopologyTuple->expirationTime + pt::seconds(T_NEIGHB_HOLD_TIME);
            mState.insertTopologyTuple(topologyTuple);
        }
    }
}

void RoutingProtocol::expireLink(const boost::system::error_code& e, boost::asio::deadline_timer *vRepeatingTimer, boost::asio::io_service *mIo,  IPv6Address& neighborAddr) {
    pt::ptime now = pt::second_clock::local_time();
    mMtxLinkExpire.lock();
    LinkTuple* vLinkTuple = mState.findLinkTuple(neighborAddr);
    mMtxLinkExpire.unlock();
    if (vLinkTuple == NULL) {
        // Maybe something else expired it already, kill the timer
        PRINTLN(Reached a state where someone else deleted a link tuple)
            vRepeatingTimer->cancel();
        delete vRepeatingTimer;
        delete mIo;
        return;
    }
    if (vLinkTuple->expirationTime < now) {
        // Remove this link
        PRINTLN(Expiring a link tuple)
        mMtxLinkExpire.lock();
        mState.cleanNeighborTuple(vLinkTuple->neighborIfaceAddr);
        mState.cleanLinkTuple(*vLinkTuple);
        mMtxLinkExpire.unlock();
        delete vRepeatingTimer;
        delete mIo;
    } else if (vLinkTuple->symTime < now) {
        PRINTLN(Updating Link tuple and its neightbor for timeout in definite expiration)
            mMtxLinkExpire.lock();
        NeighborTuple* vNeighbor = mState.findNeighborTuple(vLinkTuple->neighborIfaceAddr);
        mMtxLinkExpire.unlock();
        if (vNeighbor == NULL) {
            updateLinkTuple(vLinkTuple, W_WILL_ALWAYS);
            mMtxLinkExpire.lock();
            mState.cleanTwoHopNeighborTuples(vLinkTuple->neighborIfaceAddr);
            mState.cleanMprSelectorTuples(vLinkTuple->neighborIfaceAddr);
            mMtxLinkExpire.unlock();
            // COMPUTE MPR HERE
            updateMPRState();
            // COMPUTE ROUTING TABLE HERE
        }
        pt::time_duration expireTimer = vLinkTuple->expirationTime - now;
        vRepeatingTimer->expires_from_now(pt::seconds(expireTimer.total_seconds()));
        vRepeatingTimer->async_wait(boost::bind(&RoutingProtocol::expireLink, this, boost::asio::placeholders::error, vRepeatingTimer, mIo, neighborAddr));
    } else {
        // Reschedule, timer is good
        PRINTLN(Rescheduling timeout for link tuple)
            pt::time_duration expireTimer = vLinkTuple->expirationTime < vLinkTuple->symTime ?
                                            vLinkTuple->symTime - vLinkTuple->expirationTime
                                            : vLinkTuple->symTime - vLinkTuple->expirationTime;
        vRepeatingTimer->expires_from_now(pt::seconds(expireTimer.total_seconds()));
        vRepeatingTimer->async_wait(boost::bind(&RoutingProtocol::expireLink, this, boost::asio::placeholders::error, vRepeatingTimer, mIo, neighborAddr));
    }

}

void RoutingProtocol::updateLinkTuple(LinkTuple* vLinkEdge, uint8_t willingness) {
    pt::ptime now = pt::second_clock::local_time();
    mMtxUpdateLinkTuple.lock();
    NeighborTuple* vNeighbor = mState.findNeighborTuple(vLinkEdge->neighborIfaceAddr);
    mMtxUpdateLinkTuple.unlock();
    if (vNeighbor == NULL) {
        // Add advertised neighbor
        NeighborTuple vNewNeighbor;
        vNewNeighbor.neighborMainAddr = vLinkEdge->neighborIfaceAddr;
        vNewNeighbor.willingness = willingness;
        if (vLinkEdge->symTime >= now) {
            vNewNeighbor.status = NeighborTuple::STATUS_SYM;
        } else {
            vNewNeighbor.status = NeighborTuple::STATUS_NOT_SYM;
        }
        mMtxUpdateLinkTuple.lock();
        mState.insertNeighborTuple(vNewNeighbor);
        // Increment advertised neighbor set sequence number
        mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
        mMtxUpdateLinkTuple.unlock();
        PRINTLN(Inserted a new neighbor)
    } else {
        // Reset the symTime for this link me -> neighbor
        if (vLinkEdge->symTime >= now) {
            vNeighbor->status = NeighborTuple::STATUS_SYM;
            PRINTLN(Reset sym time to sym for neighbor)
            } else {
            vNeighbor->status = NeighborTuple::STATUS_NOT_SYM;
            PRINTLN(Reset sym time to not sym for neighbor)
            }
    }
}

void RoutingProtocol::updateMPRState() {
    // Based on RFC 3626 8.3.1.  MPR Computation
    //    The following specifies a proposed heuristic for selection of MPRs.
    //    It constructs an MPR-set that enables a node to reach any node in the
    //    symmetrical strict 2-hop neighborhood through relaying by one MPR
    //    node with willingness different from WILL_NEVER.  The heuristic MUST
    //    be applied per interface, I.  The MPR set for a node is the union of
    //    the MPR sets found for each interface.  The following terminology
    //    will be used in describing the heuristics:

    //        neighbor of an interface
    //               a node is a "neighbor of an interface" if the interface
    //               (on the local node) has a link to any one interface of
    //               the neighbor node.
    //        2-hop neighbors reachable from an interface
    //               the list of 2-hop neighbors of the node that can be
    //               reached from neighbors of this interface.
    //
    //        MPR set of an interface
    //               a (sub)set of the neighbors of an interface with a
    //               willingness different from WILL_NEVER, selected such that
    //               through these selected nodes, all strict 2-hop neighbors
    //               reachable from that interface are reachable.
    std::set<IPv6Address> vMPRs;
    //        N:
    //               N is the subset of neighbors of the node, which are
    //               neighbor of the interface I.
    mMtxMprUpdate.lock();
    std::vector<NeighborTuple> N = mState.getNeighbors();
    //        N2:
    //               The set of 2-hop neighbors reachable from the interface
    //               I, excluding:
    //                (i)   the nodes only reachable by members of N with
    //                      willingness WILL_NEVER
    //                (ii)  the node performing the computation
    //                (iii) all the symmetric neighbors: the nodes for which
    //                      there exists a symmetric link to this node on some
    //                      interface.
    std::vector<TwoHopNeighborTuple> vCurrentTwoHopNeighbors = mState.getTwoHopNeighbors();
    mMtxMprUpdate.unlock();
    std::vector<TwoHopNeighborTuple> N2;
    for (std::vector<TwoHopNeighborTuple>::iterator it = vCurrentTwoHopNeighbors.begin();
            it != vCurrentTwoHopNeighbors.end(); it++) {
        // exclusing the node performing the computation
        if (it->twoHopNeighborAddr == mPersonalAddress) {
            continue;
        }
        //    excluding        (i)   the nodes only reachable by members of N with
        //                      willingness WILL_NEVER
        bool filter2hop = false;
        for (std::vector<NeighborTuple>::iterator neigh = N.begin(); neigh != N.end(); neigh++) {
            if (neigh->neighborMainAddr == it->neighborMainAddr) {
                if (neigh->willingness == W_WILL_NEVER) {
                    filter2hop = false;
                    continue;
                } else {
                    filter2hop = true;
                    break;
                }
            }
        }
        //                (iii) all the symmetric neighbors: the nodes for which
        //                      there exists a symmetric link to this node on some
        //                      interface.
        for (std::vector<NeighborTuple>::iterator neigh = N.begin(); neigh != N.end(); neigh++) {
            if (neigh->neighborMainAddr == it->twoHopNeighborAddr) {
                filter2hop = false;
                break;
            }
        }
        // By now we have filtered the redundant neighbors.
        // (my neight has a 2h neighbor that is also another one of my neighbors)
        if(filter2hop) {
            N2.push_back(*it);
        }
    }
    //    The proposed MPR heuristic is as follows:
    //      1    Start with an MPR set made of all members of N with
    //           N_willingness equal to WILL_ALWAYS
    for(std::vector<NeighborTuple>::iterator it = N.begin(); it != N.end(); it++) {
        if(it->willingness == W_WILL_ALWAYS) {
            vMPRs.insert(it->neighborMainAddr);
            removeCoveredTwoHopNeighbor(it_>neighborMainAddr, N2);
        }
    }

    //      2    Calculate D(y), where y is a member of N, for all nodes in N.

    //      3    Add to the MPR set those nodes in N, which are the *only*
    //           nodes to provide reachability to a node in N2.  For example,
    //           if node b in N2 can be reached only through a symmetric link
    //           to node a in N, then add node a to the MPR set.  Remove the
    //           nodes from N2 which are now covered by a node in the MPR set.

    //      4    While there exist nodes in N2 which are not covered by at
    //           least one node in the MPR set:
    //           4.1  For each node in N, calculate the reachability, i.e., the
    //                number of nodes in N2 which are not yet covered by at
    //                least one node in the MPR set, and which are reachable
    //                through this 1-hop neighbor;

    //           4.2  Select as a MPR the node with highest N_willingness among
    //                the nodes in N with non-zero reachability.  In case of
    //                multiple choice select the node which provides
    //                reachability to the maximum number of nodes in N2.  In
    //                case of multiple nodes providing the same amount of
    //                reachability, select the node as MPR whose D(y) is
    //                greater.  Remove the nodes from N2 which are now covered
    //                by a node in the MPR set.

    //      5    A node's MPR set is generated from the union of the MPR sets
    //           for each interface.  As an optimization, process each node, y,
    //           in the MPR set in increasing order of N_willingness.  If all
    //           nodes in N2 are still covered by at least one node in the MPR
    //           set excluding node y, and if N_willingness of node y is
    //           smaller than WILL_ALWAYS, then node y MAY be removed from the
    //           MPR set.

    //    Other algorithms, as well as improvements over this algorithm, are
    //    possible.  For example, assume that in a multiple-interface scenario
    //    there exists more than one link between nodes 'a' and 'b'.  If node
    //    'a' has selected node 'b' as MPR for one of its interfaces, then node
    //    'b' can be selected as MPR without additional performance loss by any
    //    other interfaces on node 'a'.

    // 8.4.  Populating the MPR Selector Set

    //    The MPR selector set of a node, n, is populated by the main addresses
    //    of the nodes which have selected n as MPR.  MPR selection is signaled
    //    through HELLO messages.

}

void RoutingProtocol::removeCoveredTwoHopNeighbor(IPv6Address addr, std::vector<TwoHopNeighborTuple>& twoHopNeighbors) {
    // This function will remove all the two hop neighbors that have been covered by an MPR
    std::set<IPv6Address> uniqueRemovals;
    for(std::vector<TwoHopNeighborTuple>::iterator it=twoHopNeighbors.begin();
            it != twoHopNeighbors.end(); it++) {
        if(it->neighborMainAddr == addr) {
            uniqueRemovals.insert(it->neighborMainAddr);
        }
    }
    for(std::vector<TwoHopNeighborTuple>::iterator it=twoHopNeighbors.begin();
            it != twoHopNeighbors.end();) {
        if(uniqueRemovals.find(it->twoHopNeighborAddr) != uniqueRemovals.end()) {
            twoHopNeighbors.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingProtocol::setPersonalAddress(const IPv6Address& address) {
    mPersonalAddress.setAddressData(address.data);
}


void RoutingProtocol::routingTableComputation () {
    //  Routing Table Calculation reference to RFC 3626
    //  Procedure of calculate the routing table
    pt::ptime now = pt::second_clock::local_time();

    //  1.  All the entries from the routing table are removed.
    mTable.clear();

    //  2.  The new routing entries are added starting with the symmetric neighbors (h=1) as the destination nodes.
    mMtxRoutingTableCalc.lock();
    std::vector<NeighborTuple> &neighborSet = mState.getNeighbors();
    mMtxRoutingTableCalc.unlock();
    for (std::vector<NeighborTuple>::const_iterator it = neighborSet.begin(); it != neighborSet.end(); it++)
    {
        //  Thus, for each neighbor tuple in the neighbor set where:
        //              N_status        = SYM
        const NeighborTuple &nbTuple = *it;
        if (nbTuple.status == NeighborTuple::STATUS_SYM) {

            //  (There is a symmetric link to the neighbor), and for each associated link tuple of the neighbor node such that
            const LinkTuple *vLinkTuple = NULL;
            mMtxRoutingTableCalc.lock();
            std::vector<LinkTuple> &linkTupleSet = mState.getLinks();
            mMtxRoutingTableCalc.unlock();
            for (std::vector<LinkTuple>::const_iterator link_it = linkTupleSet.begin(); link_it != linkTupleSet.end(); link_it++)
            {
                const LinkTuple &linkTuple = *link_it;
                //  L_time >= current time, a new routing entry is recorded in the routing table with:
                if (linkTuple.symTime >= now &&
                    linkTuple.neighborIfaceAddr == nbTuple.neighborMainAddr) {
                    //              R_dest_addr     = L_neighbor_iface_add, of the associated link tuple;
                    //              R_next_addr     = L_neighbor_iface_addr, of the associated link tuple;
                    //              R_dist          = 1;
                    //              R_iface_addr    = L_local_iface_addr of the associated link tuple.
                    vLinkTuple = &linkTuple;
                    IPv6Address dest = linkTuple.neighborIfaceAddr;
                    IPv6Address next = linkTuple.neighborIfaceAddr;
                    uint32_t dist = 1;

                    RoutingTableEntry &entry = mTable[dest];
                    entry.destAddr = dest;
                    entry.nextAddr = next;
                    entry.distance = dist;
                    //  If in the above, no R_dest_addr is euqal to the main address of the neighbor, then another new routing entry
                    //  with MUST be added, with:
                    if (linkTuple.neighborIfaceAddr != nbTuple.neighborMainAddr) {
                        //              R_dest_addr     = main address of the neighbor;
                        //              R_next_addr     = L_neighbor_iface_addr of one of the associated link tuple with L_time >= current time;
                        //              R_dist          = 1;
                        //              R_iface_addr    = L_local_ifcace_addr of the associated link tuple.
                        dest = nbTuple.neighborMainAddr;
                        next = nbTuple.neighborMainAddr;

                        RoutingTableEntry &entry = mTable[dest];
                        entry.destAddr = dest;
                        entry.nextAddr = next;
                        entry.distance = dist;
                    }
                }
            }
        }
    }

    //  3.  For each node in N2, i.e., a 2-hop neighbor which is not a neighbor node or the node itself, and such that there
    const std::vector<TwoHopNeighborTuple> &twoHopNeighbors = mState.getTwoHopNeighbors();
    for (std::vector<TwoHopNeighborTuple>::const_iterator twoHopNb_it = twoHopNeighbors.begin(); twoHopNb_it != twoHopNeighbors.end(); twoHopNb_it++)
    {
        const TwoHopNeighborTuple &neighbor2HopTuple = *twoHopNb_it;
        if (!mState.findNeighborTuple(neighbor2HopTuple.twoHopNeighborAddr) || neighbor2HopTuple.twoHopNeighborAddr != mPersonalAddress) {
            //  exist at least one entry in the 2-hop neighbor set where N_neighbor_main_addr correspond to a neighbor node with
            //  willingness different of WILL_NEVER, one selects one 2-hop tuple and creates one entry in the routing table with:
            bool nb2hopOk = false;
            const std::vector<NeighborTuple> &neighborSet = mState.getNeighbors();
            for (std::vector<NeighborTuple>::const_iterator neighbor_it = neighborSet.begin(); neighbor_it != neighborSet.end(); neighbor_it++)
            {
                if (neighbor_it->neighborMainAddr == neighbor2HopTuple.twoHopNeighborAddr && neighbor_it->willingness == W_WILL_NEVER)
                {
                    nb2hopOk = true;
                    break;
                }
            }

            if (nb2hopOk) {
                RoutingTableEntry findEntry;
                IPv6Address destAddr = neighbor2HopTuple.neighborMainAddr;
                IPv6Address nextAddr = neighbor2HopTuple.twoHopNeighborAddr;
                std::map<IPv6Address, RoutingTableEntry>::const_iterator entry_it = mTable.find(destAddr);
                if (entry_it != mTable.end())
                {
                    //              R_dest_addr     = the main address of the 2-hop neighbor;
                    //              R_next_addr     = the R_next_addr of the entry in the routing table with:
                    //                                  R_dest_addr     == N_neighbor_main_addr of the 2-hop tuple;
                    //              R_dist          = 2;
                    //              R_iface_addr    = the R_iface_addr of the entry in the routing table with:
                    //                                  R_dest_addr      == N_neighbor_main_addr of the 2-hop tuple;
                    findEntry = entry_it->second;
                    RoutingTableEntry &entry = mTable[destAddr];
                    entry.destAddr = nextAddr;
                    entry.nextAddr = findEntry.nextAddr;
                    entry.distance = 2;
                }
            }
        }
    }

    for (uint32_t h = 2; ; h++)
    {
        //  The new route entries for the destiona nodes h+1 hops away are recoreded in the routing table. The following procedure
        //  MUST be executed for each value of h, starting with h=2 and incrementing it by 1 each time. The execution will stop if
        //  no new entry is recorded in an iteration.
        bool added = false;
        const std::vector<TopologyTuple> &topology = mState.getTopologySet();
        for (std::vector<TopologyTuple>::const_iterator it = topology.begin(); it != topology.end(); it++)
        {
            const TopologyTuple &topologyTuple = *it;
            //      3.1 For each topology entry in the topology table, if its T_dest_addr does not correspond to R_dest_addr of any
            //      route entry in th routing table AND its T_last_addr corresponds to R_dest_addr of a route entry whose R_dist is
            //      equal to h, then a new route entry MUST be recoreded in the routing table (if it does not already  exist) where:
            RoutingTableEntry destAddrEntry, lastAddrEntry;
            IPv6Address destAddr = topologyTuple.destAddr;
            IPv6Address lastAddr = topologyTuple.lastAddr;
            std::map<IPv6Address, RoutingTableEntry>::const_iterator entry_it = mTable.find(destAddr);
            bool foundDestAddrEntry = false;
            bool foundLastAddrEntry = false;
            if (entry_it != mTable.end()) {
                destAddrEntry = entry_it->second;
                foundDestAddrEntry = true;
            }
            entry_it = mTable.find(lastAddr);
            if (entry_it != mTable.end()) {
                lastAddrEntry = entry_it->second;
                foundLastAddrEntry = true;
            }

            if (!foundDestAddrEntry && foundLastAddrEntry)
            {
                if (lastAddrEntry.distance == h)
                {
                    //              R_dest_addr     = T_dest_addr;
                    //              R_next_addr     = R_next_addr of the recorded route entry where:
                    //                                  R_dest_addr     == T_last_addr
                    //              R_dist          = h+1; and
                    //              R_iface_addr    = R_iface_addr of the recored route entry where:
                    //                                  R_dest_addr == T_last_addr.
                    RoutingTableEntry &entry = mTable[lastAddr];
                    entry.destAddr = topologyTuple.destAddr;
                    entry.nextAddr = lastAddrEntry.nextAddr;
                    entry.distance = h + 1;
                    added = true;
                }
            }
        }

        //      3.2 Serveral topology entries may be used to select a next hop R_next_addr for reaching the node R_dest_addr.
        //      When h=1, ties should be broken such that nodes with highest willingness and MPR selectors are preferred as next hop.
        if (!added) {
            break;
        }

    }

    //  4.  For each entry in the multiple interface association base where there exists a routing entry such that:
    //              R_dest_addr     == I_main_addr (of the multiple interface association entry)
    //  And there is no routing entry such that:
    //              R_dest_addr     == I_iface_addr
    //  then a route entry is created in the routing table with:
    //              R_dest_addr     = I_iface_addr  (of the multiple interface association entry)
    //              R_next_addr     = R_next_addr   (of the recorded route entry)
    //              R_dist          = R_dist        (of the recorded route entry)
    //              R_iface_addr    = R_iface_addr  (of the recoreded route entry)
    //  No needed to implement in our project
}
