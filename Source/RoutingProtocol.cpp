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

void RoutingProtocol::handleTCMessage(TCMessage&, IPv6Address&) {}

void RoutingProtocol::handleTCMessage(std::shared_ptr<OLSRMessage> message) {

    // IPv6Address sender   = message.get()->getSender();
    // TCMessage    &tc     = message.get()->getTCMessage();

    // const LinkTuple *linkTuple = mState.findSymLinkTuple(sender);
    // if (linkTuple == NULL) return;

    // const IPv6Address lastAddr = message.get()->getOriginatorAddress();
    // TopologyTuple *topologyTuple = mState.findNewerTopologyTuple(lastAddr, tc.getAnsn());
    // if (topologyTuple != NULL) return;

    // mState.cleanOlderTopologyTuples(lastAddr, tc.getAnsn());

    // const std::vector<IPv6Address> &neighborAddresses = tc.getNeighborAddresses();
    // for(std::vector<IPv6Address>::const_iterator i = neighborAddresses.begin(); i != neighborAddresses.end(); i++)
    // {
    //  const IPv6Address &addr = *i;
    //  topologyTuple = mState.findTopologyTuple(sender, message.get()->getOriginatorAddress());
    //  if (topologyTuple != NULL){
    //      // Topology Tuple Expiration Time
    //  }else{
    //      TopologyTuple topologyTuple;
    //      topologyTuple.destAddr = addr;
    //      topologyTuple.lastAddr = message.get()->getOriginatorAddress();
    //      topologyTuple.sequenceNumber = tc.getAnsn();
    //      //topologyTuple.expirationTime = message.get()->getVTime()+1;
    //      mState.insertTopologyTuple(topologyTuple);
    //  }
    // }

    // // We may dont need it. Delete it later or move it to generateHelloMessage or readHelloMessage
    // const std::vector<TopologyTuple> & topology = mState.getTopologySet();
    // for(std::vector<TopologyTuple>::const_iterator tuple = topology.begin(); tuple != topology.end(); tuple++)
    // {}
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

}

void RoutingProtocol::setPersonalAddress(const IPv6Address& address) {
    mPersonalAddress.setAddressData(address.data);
}

// void RoutingProtocol::handleHelloMessage(std::shared_ptr<OLSRMessage> message) {
//  IPv6Address receiver = message.get()->getReceiver();
//  IPv6Address sender  = message.get()->getSender();
//  HelloMessage hello  = message.get()->getHelloMessage();
//  // LinkSensing (msg, hello, receiverIface, senderIface);

//  // We may dont need it. Delete it later or move it to generateHelloMessage or readHelloMessage
//  const std::vector<LinkTuple> &links = mState.getLinks();
//  for (std::vector<LinkTuple>::const_iterator link = links.begin(); link != links.end(); link++)
//  {}

//  // We may dont need it. Delete it later or move it to generateHelloMessage or readHelloMessage
//  const std::vector<NeighborTuple> &neighbors = mState.getNeighbors();
//  for (std::vector<NeighborTuple>::const_iterator neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++)
//  {}

//  // PopulateNeighborSet(message, hello);
//  NeighborTuple *neighbourTuple = mState.findNeighborTuple (message.get()->getOriginatorAddress());
//  if (neighbourTuple != NULL)
//  {
//       neighbourTuple->willingness = hello.getWillingness();
//  }
//  populateTwoHopNeighborSet(message, hello);

//  // We may dont need it. Delete it later or move it to generateHelloMessage or readHelloMessage
//  const std::vector<TwoHopNeighborTuple> &twoHopNeighbors = mState.getTwoHopNeighbors();
//  for (std::vector<TwoHopNeighborTuple>::const_iterator tuple = twoHopNeighbors.begin(); tuple != twoHopNeighbors.end(); tuple++)
//  {}

//  // TODO
//  // MPRComupation();
//  // PopulateMPRSelectorSet(message, hello);
// }


// void RoutingProtocol::populateTwoHopNeighborSet (const std::shared_ptr<OLSRMessage> &message, const HelloMessage &hello)
// {
//  const std::vector<LinkTuple> &links = mState.getLinks();
//  for (std::vector<LinkTuple>::const_iterator link = links.begin(); link != links.end(); link++)
//  {
//      if (getPersonalAddress(link->neighborIfaceAddr) != message.get()->getOriginatorAddress()){ continue; }
//      // Link Tuple Time Expired and Ignore to continue
//      // if (linkTuple->symTime < now) { continue; }

//      std::vector<HelloMessage::LinkMessage> linkMessages = hello.getLinkMessages();
//      for (std::vector<HelloMessage::LinkMessage>::const_iterator linkMessage = linkMessages.begin ();
//          linkMessage != linkMessages.end (); linkMessage++)
//      {
//          int neighborType= 0; // = (linkMessage->linkCode >> 2) & 0x3;
//          for (std::vector<IPv6Address>::const_iterator nb2hopAddr_iter = linkMessage->neighborIfAddr.begin();
//              nb2hopAddr_iter != linkMessage->neighborIfAddr.end(); nb2hopAddr_iter++)
//          {
//              IPv6Address neigbour2HopAddr = getPersonalAddress(*nb2hopAddr_iter);
//              if (neighborType == N_SYM_NEIGH || neighborType == N_MPR_NEIGH)
//              {
//                  // Check Neighbour Two Hop Addr and Routing Agent Addr
//                  // if (neigbour2HopAddr == m_routingAgentAddr) { continue; }

//                  TwoHopNeighborTuple *nb2hop_tuple = mState.findTwoHopNeighborTuple
//                  (message.get()->getOriginatorAddress(), neigbour2HopAddr);
//                  if (nb2hop_tuple == NULL)
//                  {
//                      TwoHopNeighborTuple newNeighbor2HopTuple;
//                      newNeighbor2HopTuple.neighborMainAddr = message.get()->getOriginatorAddress();
//                      newNeighbor2HopTuple.twoHopNeighborAddr = neigbour2HopAddr;
//                      // newNeighbor2HopTuple.expirationTime = now + message.get()->GetVTime ();
//                      mState.insertTwoHopNeighborTuple(newNeighbor2HopTuple);
//                  } else {
//                      // Two Hop Neighbor Tuple Expiration Time
//                  }
//              } else if (neighborType == N_NOT_NEIGH) {
//                  mState.cleanTwoHopNeighborTuples (message.get()->getOriginatorAddress(), neigbour2HopAddr);
//              }
//          }
//      }
//  }
// }

// RoutingProtocol::~RoutingProtocol()
// {}

// void RoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
// {
//  m_ansn = S_MAX_SEQ_NUM;
//  m_linkTupleTimerFirstTime = true;
//  m_ipv6 = ipv6;
// }

// void RoutingProtocol::SetMainInterface (uint32_t interface)
// {
//  // m_mainAddress = m_ipv6->GetAddress (interface, 0).GetLocal();
// }

// void RoutingProtocol::MprComputation()
// {
//  set<IPv6Address> mprSet;
//  std::vector<NeighborTuple> N;

//  for (std::vector<NeighborTuple>::const_iterator neighbor = m_state.GetNeighbors().begin();
//       neighbor != m_state.GetNeighbors().end(); neighbor++)
//  {
//      if (neighbor->status == NeighborTuple::STATUS_SYM) // I think that we need this check
//      {
//          N.push_back (*neighbor);
//      }
//  }

//  std::vector<TwoHopNeighborTuple> N2;
//  for (std::vector<TwoHopNeighborTuple>::const_iterator twoHopNeigh = m_state.GetTwoHopNeighbors().begin();
//       twoHopNeigh != m_state.GetTwoHopNeighbors().end(); twoHopNeigh++)
//  {
//      if (twoHopNeigh->twoHopNeighborAddr == m_mainAddress) {
//          continue;
//      }

//      bool ok = false;
//      for (std::vector<NeighborTuple>::const_iterator neigh = N.begin();
//           neigh != N.end(); neigh++)
//      {
//          if (neigh->neighborMainAddr == twoHopNeigh->neighborMainAddr)
//          {
//              if (neigh->willingness == OLSR_WILL_NEVER)
//              {
//                  ok = false;
//                  break;
//              }
//              else
//              {
//                  ok = true;
//                  break;
//              }
//          }
//      }
//      if (!ok)
//      {
//          continue;
//      }

//      // excluding:
//      // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
//      //       link to this node on some interface.
//      for (std::vector<NeighborTuple>::const_iterator neigh = N.begin();
//           neigh != N.end(); neigh++)
//      {
//          if (neigh->neighborMainAddr == twoHopNeigh->twoHopNeighborAddr)
//          {
//              ok = false;
//              break;
//          }
//      }

//      if (ok)
//      {
//          N2.push_back (*twoHopNeigh);
//      }
//  }

//  // 1. Start with an MPR set made of all members of N with
//  // N_willingness equal to WILL_ALWAYS
//  for (std::vector<NeighborTuple>::const_iterator neighbor = N.begin(); neighbor != N.end(); neighbor++)
//  {
//      if (neighbor->willingness == OLSR_WILL_ALWAYS)
//      {
//          mprSet.insert (neighbor->neighborMainAddr);
//          // (not in RFC but I think is needed: remove the 2-hop
//          // neighbors reachable by the MPR from N2)
//          for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
//               twoHopNeigh != N2.end(); )
//          {
//              if (twoHopNeigh->neighborMainAddr == neighbor->neighborMainAddr)
//              {
//                  twoHopNeigh = N2.erase (twoHopNeigh);
//              }
//              else
//              {
//                  twoHopNeigh++;
//              }
//          }
//      }
//  }

//  // 2. Calculate D(y), where y is a member of N, for all nodes in N.
//  // (we do this later)

//  // 3. Add to the MPR set those nodes in N, which are the *only*
//  // nodes to provide reachability to a node in N2.
//  std::set<Ipv6Address> coveredTwoHopNeighbors;
//  for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin(); twoHopNeigh != N2.end(); twoHopNeigh++)
//  {
//      std::vector<NeighborTuple>::const_iterator onlyNeighbor = N.end();

//      for (std::vector<NeighborTuple>::const_iterator neighbor = N.begin();
//           neighbor != N.end(); neighbor++)
//      {
//          if (neighbor->neighborMainAddr == twoHopNeigh->neighborMainAddr)
//          {
//              if (onlyNeighbor == N.end())
//              {
//                  onlyNeighbor = neighbor;
//              }
//              else
//              {
//                  onlyNeighbor = N.end();
//                  break;
//              }
//          }
//      }
//      if (onlyNeighbor != N.end())
//      {
//          mprSet.insert (onlyNeighbor->neighborMainAddr);
//          coveredTwoHopNeighbors.insert (twoHopNeigh->twoHopNeighborAddr);
//      }
//  }
//  // Remove the nodes from N2 which are now covered by a node in the MPR set.
//  for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
//       twoHopNeigh != N2.end(); )
//  {
//      if (coveredTwoHopNeighbors.find (twoHopNeigh->twoHopNeighborAddr) != coveredTwoHopNeighbors.end())
//      {
//          twoHopNeigh = N2.erase (twoHopNeigh);
//      }
//      else
//      {
//          twoHopNeigh++;
//      }
//  }

//  // 4. While there exist nodes in N2 which are not covered by at
//  // least one node in the MPR set:
//  while (N2.begin() != N2.end())
//  {
//      // 4.1. For each node in N, calculate the reachability, i.e., the
//      // number of nodes in N2 which are not yet covered by at
//      // least one node in the MPR set, and which are reachable
//      // through this 1-hop neighbor
//      std::map<int, std::vector<const NeighborTuple *> > reachability;
//      std::set<int> rs;
//      for (std::vector<NeighborTuple>::iterator it = N.begin(); it != N.end(); it++)
//      {
//          NeighborTuple const &nb_tuple = *it;
//          int r = 0;
//          for (std::vector<TwoHopNeighborTuple>::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
//          {
//              TwoHopNeighborTuple const &nb2hop_tuple = *it2;
//              if (nb_tuple.neighborMainAddr == nb2hop_tuple.neighborMainAddr)
//                  r++;
//          }
//          rs.insert (r);
//          reachability[r].push_back (&nb_tuple);
//      }

//      // 4.2. Select as a MPR the node with highest N_willingness among
//      // the nodes in N with non-zero reachability. In case of
//      // multiple choice select the node which provides
//      // reachability to the maximum number of nodes in N2. In
//      // case of multiple nodes providing the same amount of
//      // reachability, select the node as MPR whose D(y) is
//      // greater. Remove the nodes from N2 which are now covered
//      // by a node in the MPR set.
//      NeighborTuple const *max = NULL;
//      int max_r = 0;
//      for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
//      {
//          int r = *it;
//          if (r == 0)
//          {
//              continue;
//          }
//          for (std::vector<const NeighborTuple *>::iterator it2 = reachability[r].begin();
//               it2 != reachability[r].end(); it2++)
//          {
//              const NeighborTuple *nb_tuple = *it2;
//              if (max == NULL || nb_tuple->willingness > max->willingness)
//              {
//                  max = nb_tuple;
//                  max_r = r;
//              }
//              else if (nb_tuple->willingness == max->willingness)
//              {
//                  if (r > max_r)
//                  {
//                      max = nb_tuple;
//                      max_r = r;
//                  }
//                  else if (r == max_r)
//                  {
//                      if (Degree (*nb_tuple) > Degree (*max))
//                      {
//                          max = nb_tuple;
//                          max_r = r;
//                      }
//                  }
//              }
//          }
//      }

//      if (max != NULL)
//      {
//          mprSet.insert (max->neighborMainAddr);
//          for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
//               twoHopNeigh != N2.end(); )
//          {
//              if (twoHopNeigh->neighborMainAddr == max->neighborMainAddr)
//              {
//                  twoHopNeigh = N2.erase (twoHopNeigh);
//              }
//              else
//              {
//                  twoHopNeigh++;
//              }
//          }
//      }
//  }
//  m_state.SetMprSet (mprSet);
// }

// Ipv6Address
// RoutingProtocol::setPersonalAddress (Ipv6Address iface_addr) const
// {
//  const InterfaceAssociationTuple *tuple = m_state.FindIfaceAssocTuple (iface_addr);

//  if (tuple != NULL)
//      return tuple->mainAddr;
//  return iface_addr;
// }

// void RoutingProtocol::RoutingTableComputation()
// {
//  // 1. All the entries from the routing table are removed.
//  m_table.clear();

//  // 2. The new routing entries are added starting with the symmetric neighbors (h=1) as the destination nodes.
//  const std::vector<NeighborTuple> &neighborSet = m_state.GetNeighbors();
//  for (std::vector<NeighborTuple>::const_iterator it = neighborSet.begin(); it != neighborSet.end(); it++)
//  {
//      NeighborTuple const &nb_tuple = *it;
//      if (nb_tuple.status == NeighborTuple::STATUS_SYM)
//      {
//          bool nb_main_addr = false;
//          const LinkTuple *lt = NULL;
//          const std::vector<LinkTuple> &linkSet = m_state.GetLinks();
//          for (std::vector<LinkTuple>::const_iterator it2 = linkSet.begin(); it2 != linkSet.end(); it2++)
//          {
//              LinkTuple const &link_tuple = *it2;
//              if ((GetMainAddress (link_tuple.neighborIfaceAddr) == nb_tuple.neighborMainAddr))
//              {
//                  lt = &link_tuple;
//                  AddEntry (link_tuple.neighborIfaceAddr, link_tuple.neighborIfaceAddr, link_tuple.localIfaceAddr, 1);
//                  if (link_tuple.neighborIfaceAddr == nb_tuple.neighborMainAddr)
//                  {
//                      nb_main_addr = true;
//                  }
//              }
//          }

//          // If, in the above, no R_dest_addr is equal to the main address of the neighbor,
//          // then another new routing entry with MUST be added, with:
//          //      R_dest_addr = main address of the neighbor;
//          //      R_next_addr = L_neighbor_iface_addr of one of the associated link tuple with L_time >= current time;
//          //      R_dist       = 1;
//          //      R_iface_addr = L_local_iface_addr of the associated link tuple.
//          if (!nb_main_addr && lt != NULL)
//          {
//              AddEntry(nb_tuple.neighborMainAddr, lt->neighborIfaceAddr, lt->localIfaceAddr, 1);
//          }
//      }
//  }

//  //  3. for each node in N2, i.e., a 2-hop neighbor which is not a neighbor node or the node itself,
//  //  and such that there exist at least one entry in the 2-hop neighbor set where
//  //  N_neighbor_main_addr correspond to a neighbor node with willingness different of WILL_NEVER,
//  const std::vector<TwoHopNeighborTuple> &twoHopNeighbors = m_state.GetTwoHopNeighbors();
//  for (std::vector<TwoHopNeighborTuple>::const_iterator it = twoHopNeighbors.begin(); it != twoHopNeighbors.end(); it++)
//  {
//      TwoHopNeighborTuple const &nb2hop_tuple = *it;
//      // a 2-hop neighbor which is not a neighbor node or the node itself
//      if (m_state.FindNeighborTuple(nb2hop_tuple.twoHopNeighborAddr)) { continue; }
//      if (nb2hop_tuple.twoHopNeighborAddr == m_mainAddress) { continue; }

//      // ...and such that there exist at least one entry in the 2-hop neighbor set
//      // where N_neighbor_main_addr correspond to a neighbor node with willingness different of WILL_NEVER...
//      bool nb2hopOk = false;
//      for (std::vector<NeighborTuple>::const_iterator neighbor = neighborSet.begin(); neighbor != neighborSet.end(); neighbor++)
//      {
//          if (neighbor->neighborMainAddr == nb2hop_tuple.neighborMainAddr && neighbor->willingness != OLSR_WILL_NEVER)
//          {
//              nb2hopOk = true;
//              break;
//          }
//      }
//      if (!nb2hopOk) { continue; }

//      // one selects one 2-hop tuple and creates one entry in the routing table with:
//      //      R_dest_addr =   the main address of the 2-hop neighbor;
//      //      R_next_addr = the R_next_addr of the entry in the routing table with:
//      //                       R_dest_addr == N_neighbor_main_addr of the 2-hop tuple;
//      //      R_dist       = 2;
//      //      R_iface_addr = the R_iface_addr of the entry in the routing table with:
//      //                       R_dest_addr == N_neighbor_main_addr of the 2-hop tuple;
//      RoutingTableEntry entry;
//      bool foundEntry = Lookup (nb2hop_tuple.neighborMainAddr, entry);

//      if (foundEntry) {
//          AddEntry (nb2hop_tuple.twoHopNeighborAddr, entry.nextAddr, entry.interface, 2);
//      }
//  }

//  for (uint32_t h = 2; ; h++)
//  {
//      bool added = false;

//      // 3.1. For each topology entry in the topology table, if its T_dest_addr does not correspond
//      // to R_dest_addr of any route entry in the routing table AND its T_last_addr corresponds to
//      // R_dest_addr of a route entry whose R_dist is equal to h, then a new route entry MUST be recorded in
//      // the routing table (if it does not already exist)
//      const std::vector<TopologyTuple> &topology = m_state.GetTopologySet();
//      for (std::vector<TopologyTuple>::const_iterator it = topology.begin(); it != topology.end(); it++)
//      {
//          const TopologyTuple &topology_tuple = *it;

//          RoutingTableEntry destAddrEntry, lastAddrEntry;
//          bool have_destAddrEntry = Lookup (topology_tuple.destAddr, destAddrEntry);
//          bool have_lastAddrEntry = Lookup (topology_tuple.lastAddr, lastAddrEntry);
//          if (!have_destAddrEntry && have_lastAddrEntry && lastAddrEntry.distance == h)
//          {
//              // then a new route entry MUST be recorded in
//              // the routing table (if it does not already exist) where:
//              //      R_dest_addr = T_dest_addr;
//              //      R_next_addr = R_next_addr of the recorded route entry where:
//              //                      R_dest_addr == T_last_addr
//              //      R_dist       = h+1;
//              //  and
//              //      R_iface_addr = R_iface_addr of the recorded
//              //  route entry where:
//              //      R_dest_addr == T_last_addr.
//              AddEntry (topology_tuple.destAddr, lastAddrEntry.nextAddr, lastAddrEntry.interface, h + 1);
//              added = true;
//          }
//      }

//      if (!added) break;
//  }

//  // 4. For each entry in the multiple interface association base where there exists a routing entry
//  // such that:                               R_dest_addr == I_main_addr  (of the multiple interface association entry)
//  // AND there is no routing entry such that: R_dest_addr == I_iface_addr
//  const std::vector<InterfaceAssociationTuple> &ifaceAssocSet = m_state.GetIfaceAssocSet();
//  for (std::vector<InterfaceAssociationTuple>::const_iterator it = ifaceAssocSet.begin(); it != ifaceAssocSet.end(); it++)
//  {
//      InterfaceAssociationTuple const &tuple = *it;
//      RoutingTableEntry entry1, entry2;
//      bool have_entry1 = Lookup (tuple.mainAddr, entry1);
//      bool have_entry2 = Lookup (tuple.ifaceAddr, entry2);
//      if (have_entry1 && !have_entry2)
//      {
//          // then a route entry is created in the routing table with:
//          //      R_dest_addr     =   I_iface_addr (of the multiple interface association entry)
//          //      R_next_addr     =   R_next_addr  (of the recorded route entry)
//          //      R_dist          =   R_dist       (of the recorded route entry)
//          //      R_iface_addr    =   R_iface_addr (of the recorded route entry).
//          AddEntry (tuple.ifaceAddr, entry1.nextAddr, entry1.interface, entry1.distance);
//      }
//  }


