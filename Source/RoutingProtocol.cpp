#include "Headers/RoutingProtocol.h"

RoutingProtocol& RoutingProtocol::updateState(std::shared_ptr<OLSRMessage> message) {
    std::cout << "RoutingProtocol::updateState: Received the message from " << message->mSenderHWAddr << " with originator " << *(message->mOriginatorAddress) << std::endl;

    mRequiresForwarding = false;
    bool vTryForward = false;
    // First check if this message has been handled before
    if (message->messages.size() == 0) {
        std::cerr << "Oh no! A message was null!" << std::endl;
        return getInstance();
    }
    mMtxDuplicate.lock();
    DuplicateTuple* vDuplicate = mState.findDuplicateTuple(*(message->mOriginatorAddress), message->mPacketSequenceNumber);
    mMtxDuplicate.unlock();

    if (vDuplicate == NULL) {
        // Here we want to handle the message
        for (std::vector<std::shared_ptr<Message>>::iterator it = message->messages.begin();
                it != message->messages.end(); it++) {
            #if verbose
            std::cout << "RoutingProtocol::updateState: Processing message of type " << (*it)->getType() << std::endl;
            #endif
            switch ((*it)->getType()) {
            case M_HELLO_MESSAGE :
                // Dereferenced HelloMsg from the address of the dereference iterator msg
                #if verbose
                PRINTLN(Handling hello msg)
                #endif
                handleHelloMessage(*((HelloMessage*) & (**it)), message->mSenderHWAddr, (*it)->mMessageHeader.vtime);
                #if verbose
                std::cout << "RoutingProtocol::updateState finished handling hello" << std::endl;
                #endif
                break;
            case M_TC_MESSAGE:
                // Handle a TC
                #if verbose
                PRINTLN(Handling tc msg)
                #endif
                handleTCMessage(*((TCMessage*) & (**it)), message->mSenderHWAddr);
                #if verbose
                std::cout << "RoutingProtocol::updateState finished handling tc" << std::endl;
                #endif
                vTryForward = true;
                break;
            case M_MID_MESSAGE:
                // Not implemented
                break;
            case M_HNA_MESSAGE:
                // Not implemented
                break;
            }
        }
    } else {
        for (std::vector<MACAddress>::const_iterator it = vDuplicate->ifaceList.begin ();
                it != vDuplicate->ifaceList.end(); it++) {
            if (*it == message->mRecvedHWAddr) {
                vTryForward = false;
                break;
            }
        }
    }
    if (vTryForward) {
        mRequiresForwarding = determineRequiresForwarding(message, vDuplicate);
        #if verbose
        std::cout << "RoutingProtocol::updateState forwarding TC packet: " << mRequiresForwarding << std::endl;
        #endif
    }
    return getInstance();
}

bool RoutingProtocol::needsForwarding() {
    return mRequiresForwarding;
}

bool RoutingProtocol::determineRequiresForwarding(std::shared_ptr<OLSRMessage> message, DuplicateTuple* duplicate) {
    pt::ptime now = pt::second_clock::local_time();
    mMtxSymLink.lock();
    LinkTuple* vLinkTuple = mState.findSymLinkTuple(message->mSenderHWAddr, now);
    mMtxSymLink.unlock();
    // If the sender cannot be reached, ie, link not symteric, then return now
    if (vLinkTuple == NULL) {
        #if verbose
        std::cout << "RoutingProtocol::determineRequiresForwarding Link to forward TC message was not found." << std::endl;
        #endif
        return false;
    }
    // If we have forward this message already, then we will make sure stop do not forward it twice
    if (duplicate != NULL && duplicate->retransmitted) {
        return false;
    }
    bool retransmitted = false;
    // Bring the message into its true form. We only deal with TC at the moment
    TCMessage& vTCMessage = *((TCMessage*) & (*(message->messages[0])));

    if (vTCMessage.mMessageHeader.timeToLive > 1) {
        mMtxMprSelector.lock();
        MprSelectorTuple* vMPRSelectors = mState.findMprSelectorTuple(message->mSenderHWAddr);
        mMtxMprSelector.unlock();
        if (vMPRSelectors != NULL) {
            vTCMessage.mMessageHeader.timeToLive -= 1;
            vTCMessage.mMessageHeader.hopCount += 1;
            retransmitted = true;
        }
    }
    if (duplicate != NULL) {
        // Set this message as non-forwardable
        duplicate->expirationTime = now + pt::seconds(T_DUP_HOLD_TIME);
        duplicate->retransmitted = retransmitted;
        duplicate->ifaceList.push_back(message->mRecvedHWAddr);
    } else {
        // First time we got this message! So lets forward her
        DuplicateTuple newDup;
        newDup.address = *(vTCMessage.getOriginatorAddress());
        newDup.sequenceNumber = message->mPacketSequenceNumber;
        newDup.expirationTime = now + pt::seconds(T_DUP_HOLD_TIME);
        newDup.retransmitted = retransmitted;
        newDup.ifaceList.push_back(message->mRecvedHWAddr);
        mMtxDuplicate.lock();
        mState.insertDuplicateTuple(newDup);
        mMtxDuplicate.unlock();
        // Schedule dup tuple deletion
        boost::thread vExpiryThread = boost::thread(boost::bind(&RoutingProtocol::expireDuplicateTC, this, T_DUP_HOLD_TIME, newDup.address, newDup.sequenceNumber));
    }
    return true;
}

void RoutingProtocol::expireDuplicateTC(int seconds, MACAddress address, uint16_t sequenceNumber) {
    sleep(seconds);
    #if verbose
    std::cout << "RoutingProtocol::expireDuplicateTC Expiry record started with TC forwarding seqnum: " << sequenceNumber
              << ", address " << address << " from our interface" << std::endl;
    #endif
    while (1) {
        pt::ptime now = pt::second_clock::local_time();
        mMtxDuplicate.lock();
        DuplicateTuple *tuple = mState.findDuplicateTuple (address, sequenceNumber);
        mMtxDuplicate.unlock();
        if (tuple == NULL) {
            #if verbose
            std::cout << "30 second passed, clearing record of TC" << std::endl;
            #endif
            return;
        }
        if (tuple->expirationTime < now) {
            #if verbose
            std::cout << "30 second passed, clearing record of TC" << std::endl;
            #endif
            mMtxDuplicate.lock();
            mState.eraseDuplicateTuple(*tuple);
            mMtxDuplicate.unlock();
            return;
        }
        else {
            pt::time_duration expireTimer = tuple->expirationTime - now;
            if (expireTimer.total_seconds() > 0) {
                #if verbose
                std::cout << "Record of TC was refreshed since last timer and has " << seconds << " seconds left" << std::endl;
                #endif
                sleep(expireTimer.total_seconds());
            } else {
                #if verbose
                std::cout << "30 second passed, clearing record of TC" << std::endl;
                #endif
                mMtxDuplicate.lock();
                mState.eraseDuplicateTuple(*tuple);
                mMtxDuplicate.unlock();
                return;
            }
        }
    }
}

int RoutingProtocol::buildHelloMessage(OLSRMessage & message) {
    pt::ptime now = pt::second_clock::local_time();
    mMtxNeighbor.lock();
    std::vector<NeighborTuple> neighbors = mState.getNeighbors();
    mMtxNeighbor.unlock();
    mMtxLink.lock();
    std::vector<LinkTuple> mLinks = mState.getLinks();
    mMtxLink.unlock();

    std::shared_ptr<HelloMessage> helloMessage = std::make_shared<HelloMessage>();
    helloMessage->mMessageHeader.vtime = T_NEIGHB_HOLD_TIME;
    // We can ommit message size as its calculated on serialization
    // helloMessage->mMessageHeader.messageSize
    memcpy(helloMessage->mMessageHeader.originatorAddress, mPersonalAddress.data, WLAN_ADDR_LEN);
    helloMessage->mMessageHeader.timeToLive = 1;
    helloMessage->mMessageHeader.hopCount = 0;
    mSequenceNumber = (mSequenceNumber + 1) % 65530;
    message.mPacketSequenceNumber = mSequenceNumber;
    helloMessage->mMessageHeader.messageSequenceNumber = mSequenceNumber;
    helloMessage->htime = 3 * T_HELLO_INTERVAL / 1000;

    for (auto& link : mLinks) {
        // If they are not supposed to link to me
        if (!(link.localIfaceAddr == mPersonalAddress && link.expirationTime >= now)) {
            #if verbose
            PRINTLN(Skipped a host when making a hello msg)
            #endif
            continue;
        }
        uint8_t linkType, neighborType = 0xff;
        // Establishes link type
        if (link.symTime >= now) {
            linkType = L_SYM_LINK;
        } else if (link.asymTime >= now) {
            linkType = L_ASYM_LINK;
        } else {
            linkType = L_LOST_LINK;
        }
        mMtxMpr.lock();
        bool found = mState.findMprAddress(link.neighborIfaceAddr);
        mMtxMpr.unlock();
        if (found) {
            neighborType = N_MPR_NEIGH;
            #if verbose
            std::cout << "RoutingProtocol::buildHelloMessage Creating hello msg and setting " << link.neighborIfaceAddr << " who choose me as a MPR candidate" << std::endl;
            #endif
        } else {
            bool ok = false;
            for (auto& neighbor : neighbors) {
                if (neighbor.neighborMainAddr == link.neighborIfaceAddr) {
                    if (neighbor.status == NeighborTuple::STATUS_SYM) {
                        neighborType = N_SYM_NEIGH;
                        #if verbose
                        std::cout << "RoutingProtocol::buildHelloMessage Creating hello msg and setting " << link.neighborIfaceAddr
                                  << " as a regular neighbor" << std::endl;
                        #endif
                    }
                    else if (neighbor.status == NeighborTuple::STATUS_NOT_SYM) {
                        neighborType = N_NOT_NEIGH;
                        #if verbose
                        std::cout << "RoutingProtocol::buildHelloMessage Creating hello msg and setting " << link.neighborIfaceAddr
                                  << " as not a bidirectional neighbor" << std::endl;
                        #endif
                    }
                    else {
                        #if verbose
                        PRINTLN(Unable to find a neighbor)
                        #endif
                    }
                    ok = true;
                    break;
                }
            }
            if (!ok) {
                continue;
            }
        }
        HelloMessage::LinkMessage vNeighborLink;
        vNeighborLink.linkCode = (linkType & 0x03) | ((neighborType << 2) & 0x0f);
        vNeighborLink.neighborIfAddr.push_back(link.neighborIfaceAddr);
        helloMessage->mLinkMessages.push_back(vNeighborLink);
    }
    message.messages.push_back(helloMessage);
    return 1;
}

int RoutingProtocol::buildTCMessage(OLSRMessage & message) {
    mMtxMprSelector.lock();
    std::vector<MprSelectorTuple> neighbors = mState.getMprSelectors();
    mMtxMprSelector.unlock();

    std::shared_ptr<TCMessage> tcMessage = std::make_shared<TCMessage>();

    tcMessage->mMessageHeader.vtime = T_NEIGHB_HOLD_TIME;
    // We can ommit message size as its calculated on serialization
    memcpy(tcMessage->mMessageHeader.originatorAddress, mPersonalAddress.data, WLAN_ADDR_LEN);
    tcMessage->mMessageHeader.timeToLive = 255;
    tcMessage->mMessageHeader.hopCount = 0;
    mSequenceNumber = (mSequenceNumber + 1) % 65530;
    message.mPacketSequenceNumber = mSequenceNumber;
    tcMessage->mMessageHeader.messageSequenceNumber = mSequenceNumber;

    tcMessage->ansn = mANSN;
    for (auto& neighborMprs : neighbors) {
        tcMessage->mNeighborAddresses.push_back(neighborMprs.mainAddr);
    }
    message.messages.push_back(tcMessage);
    return 1;
}

void RoutingProtocol::handleHelloMessage(HelloMessage & message, const MACAddress & senderHWAddr, unsigned char vtime) {
    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage: Process hello message and update state" << std::endl;
    #endif
    mMtxLink.lock();
    LinkTuple* vLinkEdge = mState.findLinkTuple(senderHWAddr);
    mMtxLink.unlock();
    bool create = false;
    bool update = false;
    pt::ptime now = pt::second_clock::local_time();

    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage: Check the neighbor from coming hello message" << std::endl;
    #endif
    if (vLinkEdge == NULL) {
        #if verbose
        PRINTLN(RoutingProtocol::handleHelloMessage: Found a new neighbor from hello msg)
        #endif
        LinkTuple newNeighbor;
        newNeighbor.neighborIfaceAddr = senderHWAddr;
        newNeighbor.localIfaceAddr = mPersonalAddress;
        newNeighbor.symTime = now - pt::seconds(1); // Provide the expire time as of now (update later)
        newNeighbor.expirationTime = now + pt::seconds(vtime); // temp
        mMtxLink.lock();
        vLinkEdge = &mState.insertLinkTuple(newNeighbor);
        mMtxLink.unlock();
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage: Added a new link into our state" << std::endl;
        #endif
        create = true;
    } else {
        #if verbose
        PRINTLN(RoutingProtocol::handleHelloMessage: Recv a hello from an existing neighbor)
        #endif
        update = true;
    }
    // Assume asymetric 4.2.1
    vLinkEdge->asymTime = now + pt::seconds(vtime);

    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage: Processing the willingness of the neighbor if exist" << std::endl;
    #endif
    // This step is unessary, but will do anyways 8.1.1 (processing willingness)
    mMtxNeighbor.lock();
    NeighborTuple *neighbor = mState.findNeighborTuple(*(message.getOriginatorAddress()));
    mMtxNeighbor.unlock();

    if (neighbor != NULL)
    {
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage: The neighbor is the same as originator then update the willingness of the neighbor" << std::endl;
        #endif
        neighbor->willingness = message.willingness;
    }

    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage: Update the expire time of the link soon if asymTime is greater" << std::endl;
    #endif
    // Will expire this link soon if asymTime is greater
    vLinkEdge->expirationTime = std::max(vLinkEdge->expirationTime, vLinkEdge->asymTime);
    #if verbose
    pt::time_duration eTimer = vLinkEdge->expirationTime - now;
    std::cout << "RoutingProtocol::handleHelloMessage the sender's expiration time is " << eTimer.total_seconds() << std::endl;
    #endif
    // Update the changes we made on this edge
    if (update) {
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage: Update the link to the link tuple set" << std::endl;
        #endif
        updateLinkTuple(vLinkEdge, message.willingness);
    }

    if (create && vLinkEdge != NULL) {
        #if verbose
        std::cout << vLinkEdge->expirationTime << " " << vLinkEdge->symTime  << std::endl;
        #endif
        pt::time_duration expireTimer = vLinkEdge->expirationTime < vLinkEdge->symTime ?
                                        vLinkEdge->symTime - vLinkEdge->expirationTime
                                        : vLinkEdge->expirationTime - vLinkEdge->symTime ;
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage: Schedule a " << expireTimer.total_seconds() << " seconds to do expire this link" << std::endl;
        #endif
        boost::thread vExpiryThread = boost::thread(boost::bind(&RoutingProtocol::expireLink, this, expireTimer.total_seconds(), vLinkEdge->neighborIfaceAddr));

    }

    // Being computing MPR and bunch of other stuff
    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage: Computing MPR and update routing state" << std::endl;
    #endif
    for (std::vector<HelloMessage::LinkMessage>::iterator linkNeighbrs = message.mLinkMessages.begin();
            linkNeighbrs != message.mLinkMessages.end(); linkNeighbrs++ ) {
        int linkType = linkNeighbrs->linkCode & 0x03; // ^C Terminiation
        int neighborType = (linkNeighbrs->linkCode >> 2) & 0x03;
        #if verbose
        std::cout << "RoutingProtocol::handle HelloMessage: Hello msg with Link Type: " << linkType << " and Neighbor Type: " << neighborType << std::endl;
        #endif
        // Skip invalid link and neightbor codes RFC 6.1.1
        if ((linkType == L_SYM_LINK && neighborType == N_NOT_NEIGH) ||
                (neighborType != N_SYM_NEIGH && neighborType != N_MPR_NEIGH
                 && neighborType != N_NOT_NEIGH)) {
            // Possible two hop neigher addrs or totally invalid
            #if verbose
            PRINTLN(RoutingProtocol::handleHelloMessage: Found invalid link type and skipping)
            #endif
            continue;
        }

        // Process the advertised neighbors
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage: Process the advertised neighbors" << std::endl;
        #endif
        for (std::vector<MACAddress>::iterator it = linkNeighbrs->neighborIfAddr.begin();
                it != linkNeighbrs->neighborIfAddr.end(); it++) {
            if (*it == mPersonalAddress) {
                if (!update) {
                    if (linkType == L_LOST_LINK) {
                        // Happens if we have gotten a msg from a neighbor that lost us.
                        vLinkEdge->symTime = now - pt::seconds(1);
                        #if verbose
                        PRINTLN(RoutingProtocol::handleHelloMessage: An advertised neighbor sent a link lost in hello message)
                        #endif
                        update = true;
                    } else if (linkType == L_ASYM_LINK || linkType == L_SYM_LINK) {
                        // Since we have just sensed the neighbor reached us
                        #if verbose
                        PRINTLN(RoutingProtocol::handleHelloMessage: A neightbor has sensed us and link between us is bi directional)
                        #endif
                        vLinkEdge->symTime = now + pt::seconds(vtime);
                        vLinkEdge->expirationTime = vLinkEdge->symTime  + pt::seconds(T_NEIGHB_HOLD_TIME);
                        update = true;
                    } else {
                        #if verbose
                        PRINTLN(RoutingProtocol::handleHelloMessage: Corrupt link type in hello msg)
                        #endif
                    }
                }
            } else {
                #if verbose
                PRINTLN(RoutingProtocol::handleHelloMessage: Found a address in hello msg that was not supposed to be for us)
                #endif
                }
        }
    }

    #if verbose
    PRINTLN(RoutingProtocol::handleHelloMessage begin examining two hop neighbor)
    #endif
    mMtxLink.lock();
    std::vector<LinkTuple> vLinks = mState.getLinks();
    mMtxLink.unlock();
    // Populate two hop neighbor set
    for (auto& link : vLinks) {

        if (link.neighborIfaceAddr != *(message.getOriginatorAddress())) {
            #if verbose
            std::cout << "RoutingProtocol::handleHelloMessage determined " << link.neighborIfaceAddr
                      << " as not a two hop neightbor of " << *(message.getOriginatorAddress()) << std::endl;
            #endif
            continue;
        }

        // if (link.symTime < now) {
        //     PRINTLN(Found an expired link tuple while trying to update the two hop neighbors)
        //         continue;
        // }
        #if verbose
        std::cout<< "RoutingProtocol::handleHelloMessage number of links on this message from "<< *(message.getOriginatorAddress()) <<"is " << message.mLinkMessages.size();
        #endif
        for (auto& linkMessage : message.mLinkMessages) {
            int neighborType = (linkMessage.linkCode >> 2) & 0x3;
            #if verbose
            std::cout << "Link message has neighbor type " << neighborType << std::endl;
            #endif
            for (auto& neighborMacAddr : linkMessage.neighborIfAddr)

                if (neighborType == N_SYM_NEIGH || neighborType == N_MPR_NEIGH) {
                    // If the main address of the 2-hop neighbor address == main address
                    // of the receiving node, silently discard the 2-hop
                    // neighbor address.
                    if (neighborMacAddr == mPersonalAddress) {
                        continue;
                    }
                    mMtxTwoHopNeighbor.lock();
                    TwoHopNeighborTuple* vTwoHopNeighbor = mState.findTwoHopNeighborTuple(*(message.getOriginatorAddress()), neighborMacAddr);
                    mMtxTwoHopNeighbor.unlock();
                    // Otherwise, a 2-hop tuple is created

                    if (vTwoHopNeighbor == NULL) {
                        #if verbose
                        std::cout << "RoutingProtocol::handleHelloMessage Added two hop neighbor: " << message.getOriginatorAddress() << std::endl;
                        #endif
                        TwoHopNeighborTuple vNewTwoHopNeighbor;
                        vNewTwoHopNeighbor.neighborMainAddr = *(message.getOriginatorAddress());
                        vNewTwoHopNeighbor.twoHopNeighborAddr = neighborMacAddr;
                        vNewTwoHopNeighbor.expirationTime = now + pt::seconds(message.mMessageHeader.vtime);
                        mMtxTwoHopNeighbor.lock();
                        mState.insertTwoHopNeighborTuple(vNewTwoHopNeighbor);
                        mMtxTwoHopNeighbor.unlock();
                        mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
                        pt::time_duration expireTimer = vNewTwoHopNeighbor.expirationTime - now;
                        // Schedules nb2hop tuple deletion
                        boost::thread vNeighborExpiryThread = boost::thread(boost::bind(
                                &RoutingProtocol::expireTwoHopNeighbor, this, expireTimer.total_seconds(),
                                vNewTwoHopNeighbor.neighborMainAddr,
                                vNewTwoHopNeighbor.twoHopNeighborAddr));
                    } else {
                        #if verbose
                        PRINTLN("RoutingProtocol::handleHelloMessage Updated expiry time of the two hop neighbor")
                        #endif
                        vTwoHopNeighbor->expirationTime = now + pt::seconds(message.mMessageHeader.vtime);
                    }
                } else if (neighborType == N_NOT_NEIGH) {
                    // For each 2-hop node listed in the HELLO message
                    // with Neighbor Type equal to NOT_NEIGH all 2-hop
                    // tuples where: N_neighbor_main_addr == Originator
                    // Address AND N_2hop_addr == main address of the
                    // 2-hop neighbor are deleted.
                    #if verbose
                    PRINTLN(Neighbor type not a neighbor now removing the instance)
                    #endif
                    mMtxTwoHopNeighbor.lock();
                    mState.cleanTwoHopNeighborTuples(*(message.getOriginatorAddress()), neighborMacAddr);
                    mMtxTwoHopNeighbor.unlock();
                } else {
                    #if verbose
                    PRINTLN("Cannot resolve neighbor type")
                    #endif
                }
        }
    }
    updateMPRState();
    // Add our MPR selectors after we update the MPR state
    #if verbose
    std::cout << "RoutingProtocol::handleHelloMessage message link msg size "
        << message.mLinkMessages.size() << std::endl;
    #endif

    for (auto& linkMessage : message.mLinkMessages) {
        int linkCode = linkMessage.linkCode >> 2;
        #if verbose
        std::cout << "RoutingProtocol::handleHelloMessage Link message has link type " << linkCode << std::endl;
        #endif
        if (linkCode == N_MPR_NEIGH) {
            #if verbose
            std::cout << "RoutingProtocol::handleHelloMessage MPR selection begins" << std::endl;
            #endif
            for (auto& neighborMacAddr : linkMessage.neighborIfAddr) {
                if (neighborMacAddr == mPersonalAddress) {

                    // We must create a new entry into the mpr selector set
                    mMtxMprSelector.lock();
                    MprSelectorTuple *vMPRSelected = mState.findMprSelectorTuple(*(message.getOriginatorAddress()));
                    mMtxMprSelector.unlock();
                    if (vMPRSelected == NULL) {
                        #if verbose
                        std::cout << "RoutingProtocol::handleHelloMessage A neighbor has selected me as an MPR! What an honour!" << std::endl;
                        #endif
                        MprSelectorTuple vNewMprSelector;
                        vNewMprSelector.mainAddr = *(message.getOriginatorAddress());
                        vNewMprSelector.expirationTime = now + pt::seconds(message.mMessageHeader.vtime);
                        mMtxMprSelector.lock();
                        mState.insertMprSelectorTuple(vNewMprSelector);
                        mMtxMprSelector.unlock();
                        pt::time_duration expireTimer = vNewMprSelector.expirationTime - now;
                        // Schedules mpr selector tuple deletion
                        boost::thread vExpireMPR = boost::thread(boost::bind(
                                                       &RoutingProtocol::expireMprSelector, this, expireTimer.total_seconds(),
                                                       vNewMprSelector.mainAddr));
                    } else {
                        #if verbose
                        std::cout << "RoutingProtocol::handleHelloMessage Heard back from my MPR selector, updating expiration time!" << std::endl;
                        #endif
                        vMPRSelected->expirationTime = now + pt::seconds(message.mMessageHeader.vtime);
                    }
                }
            }
        }
    }
}

void RoutingProtocol::expireMprSelector(int seconds, MACAddress addressSelector) {
    sleep(seconds);
    while (1) {
        pt::ptime now = pt::second_clock::local_time();
        mMtxMprSelector.lock();
        MprSelectorTuple *vMprSelector = mState.findMprSelectorTuple (addressSelector);
        mMtxMprSelector.unlock();
        if (vMprSelector == NULL) {
            #if verbose
            std::cout << "RoutingProtocol::expireMprSelector MPR selector " << addressSelector << " is expired." << std::endl;
            #endif
            return;
        }
        if (vMprSelector->expirationTime < now) {
            #if verbose
            std::cout << "RoutingProtocol::expireMprSelector MPR selector " << addressSelector << " is expired." << std::endl;
            #endif
            mMtxMprSelector.lock();
            mState.cleanMprSelectorTuple(*vMprSelector);
            mMtxMprSelector.unlock();
            mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
            return;
        } else {
            #if verbose
            std::cout << "RoutingProtocol::expireMprSelector MPR selector " << addressSelector << " time got a refresh for " << seconds << std::endl;
            #endif
            pt::time_duration expireTimer = vMprSelector->expirationTime - now;
            if(expireTimer.total_seconds() == 0) {
                #if verbose
                std::cout << "RoutingProtocol::expireMprSelector MPR selector " << addressSelector << " is expired." << std::endl;
                #endif
                mMtxMprSelector.lock();
                mState.cleanMprSelectorTuple(*vMprSelector);
                mMtxMprSelector.unlock();
                mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
                return;
            }
            sleep(expireTimer.total_seconds());
        }
    }
}

void RoutingProtocol::expireTwoHopNeighbor(int seconds, MACAddress neighborAddr, MACAddress twoHopNeighbor) {
    sleep(seconds);

    while (1) {
        pt::ptime now = pt::second_clock::local_time();
        mMtxTwoHopNeighbor.lock();
        TwoHopNeighborTuple *twoHopTuple = mState.findTwoHopNeighborTuple(neighborAddr, twoHopNeighbor);
        mMtxTwoHopNeighbor.unlock();
        if (twoHopTuple == NULL) {
            #if verbose
            std::cout << "RoutingProtocol::expireTwoHopNeighbor TwoHopNeighbor " << twoHopNeighbor << " is expired." << std::endl;
            #endif
            return;
        }
        if (twoHopTuple->expirationTime < now) {
            #if verbose
            std::cout << "RoutingProtocol::expireTwoHopNeighbor TwoHopNeighbor " << twoHopNeighbor << " is expired." << std::endl;
            #endif
            mMtxTwoHopNeighbor.lock();
            mState.cleanTwoHopNeighborTuple (*twoHopTuple);
            mMtxTwoHopNeighbor.unlock();
            mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
            return;
        } else {
            pt::time_duration expireTimer = twoHopTuple->expirationTime - now;
            seconds = expireTimer.total_seconds();
            if(seconds == 0) {
                #if verbose
                std::cout << "RoutingProtocol::expireTwoHopNeighbor TwoHopNeighbor " << twoHopNeighbor << " is expired." << std::endl;
                #endif
                 mMtxTwoHopNeighbor.lock();
                mState.cleanTwoHopNeighborTuple (*twoHopTuple);
                mMtxTwoHopNeighbor.unlock();
                mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
                return;
            }
            #if verbose
            std::cout << "RoutingProtocol::expireTwoHopNeighbor TwoHopNeighbor " << twoHopNeighbor << " time got a refresh for " << seconds << std::endl;
            #endif
            sleep(seconds);
        }
    }
}

void RoutingProtocol::expireLink(int seconds, MACAddress neighborAddr) {
    #if verbose
    std::cout << "RoutingProtocol::expireLink: A Link timer has been starteds, sleeping for " << seconds << std::endl;
    #endif
    sleep(seconds);
    #if verbose
    std::cout << "Waking up to check sym times for " << neighborAddr << std::endl;
    #endif
    pt::time_duration expireTimer;
    seconds = expireTimer.total_seconds();
    do {
        pt::ptime now = pt::second_clock::local_time();
        mMtxLink.lock();
        LinkTuple* vLinkTuple = mState.findLinkTuple(neighborAddr);
        mMtxLink.unlock();
        if (vLinkTuple == NULL) {
            // Maybe something else expired it already, kill the timer
            #if verbose
            std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " is expired." << std::endl;
            #endif
            return;
        }
        expireTimer = vLinkTuple->expirationTime - now;
        seconds = expireTimer.total_seconds();
        #if verbose
        std::cout << "RoutingProtocol::expireLink: Link expires in " << expireTimer << std::endl;
        #endif

        if (vLinkTuple->expirationTime < now) {
            // Remove this link
            #if verbose
            PRINTLN(RoutingProtocol::expireLink: Expiring a link tuple)
            std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " is expired." << std::endl;
            #endif
            mMtxNeighbor.lock();
            mState.cleanNeighborTuple(vLinkTuple->neighborIfaceAddr);
            mMtxNeighbor.unlock();
            mMtxLink.lock();
            mState.cleanLinkTuple(*vLinkTuple);
            mMtxLink.unlock();
            return;

        } else if (vLinkTuple->symTime < now) {
            #if verbose
            PRINTLN(RoutingProtocol::expireLink: Updating Link tuple and its neightbor for timeout in definite expiration)
            #endif

                mMtxNeighbor.lock();
            NeighborTuple* vNeighbor = mState.findNeighborTuple(vLinkTuple->neighborIfaceAddr);
            mMtxNeighbor.unlock();
            if (vNeighbor == NULL) {
                #if verbose
                std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " is expired because neighbor is null." << std::endl;
                #endif
                updateLinkTuple(vLinkTuple, W_WILL_ALWAYS);
                mMtxTwoHopNeighbor.lock();
                mState.cleanTwoHopNeighborTuples(vLinkTuple->neighborIfaceAddr);
                mMtxTwoHopNeighbor.unlock();
                mMtxMprSelector.lock();
                mState.cleanMprSelectorTuples(vLinkTuple->neighborIfaceAddr);
                mMtxMprSelector.unlock();
                updateMPRState();
                //routingTableComputation();
            }
            expireTimer = vLinkTuple->expirationTime - now;

            if (expireTimer.total_seconds() <= 0) {
                // Expire now
                #if verbose
                std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " is expired because timer < 0." << std::endl;
                #endif
                mMtxNeighbor.lock();
                mState.cleanNeighborTuple(vLinkTuple->neighborIfaceAddr);
                mMtxNeighbor.unlock();
                mMtxLink.lock();
                mState.cleanLinkTuple(*vLinkTuple);
                mMtxLink.unlock();
                return;
            }
            seconds = expireTimer.total_seconds();
            #if verbose
            std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " time got a refresh for " << seconds << " (c)" << std::endl;
            #endif
            sleep(seconds);
        } else {
            // Reschedule, timer is good
            expireTimer = vLinkTuple->expirationTime < vLinkTuple->symTime ?
                          vLinkTuple->symTime - vLinkTuple->expirationTime
                          : vLinkTuple->symTime - vLinkTuple->expirationTime;

            seconds = expireTimer.total_seconds();
            #if verbose
            std::cout << "RoutingProtocol::expireLink: Link with " << neighborAddr << " time got a refresh for " << seconds << std::endl;
            #endif
            sleep(seconds);
        }
    } while (seconds > 0);
}

void RoutingProtocol::handleTCMessage(TCMessage & message, MACAddress & senderHWAddr) {
    #if verbose
    std::cout << "RoutingProtocol::handleTCMessage: Process tc message and update state" << std::endl;
    #endif
    // Double checked with RFC 3626 with TC message processing
    // Should follow the pattern of the handling tc message will be great due to mState is already implemented
    // Either discuss for change or keep in this pattern

    //  1.  If the sender interface (NB: not originator) of this message is not in the symmetric 1-hotp neighborhodd
    //  of this node, the message MUST be discarded.
    #if verbose
    std::cout << "RoutingProtocol::handleTCMessage: Check the neighbor of this message if it is not originator else discard" << std::endl;
    #endif
    pt::ptime now = pt::second_clock::local_time();
    mMtxSymLink.lock();
    LinkTuple* linkTuple = mState.findSymLinkTuple(senderHWAddr, now);
    mMtxSymLink.unlock();
    if (linkTuple == NULL)
    {
        #if verbose
        std::cout << "RoutingProtocol::handleTCMessage: The link for this TC message was not found from sender: " << senderHWAddr << std::endl;
        #endif
        return;
    }

    //  2. If there exist some tuple in the topology set where:
    //          T_last_addr == originator address AND
    //          T_seq       >   ANSN,
    //  then further processing of this TC message MUST NOT be performed and the message MUST besilently discard
    //  (case: message received out of order).
    #if verbose
    std::cout << "RoutingProtocol::handleTCMessage: Check the tc message and discard if message received out of order" << std::endl;
    #endif
    MACAddress lastAddr =  *(message.getOriginatorAddress());
    uint16_t ansn = message.ansn;
    mMtxTopology.lock();
    TopologyTuple* vTopologyTuple = mState.findNewerTopologyTuple(lastAddr, ansn);
    mMtxTopology.unlock();
    if (vTopologyTuple != NULL)
    {
        #if verbose
        std::cout << "RoutingProtocol::handleTCMessage: Message received out of order discard this message" << std::endl;
        #endif
        return;
    }


    //  3. All tuples in the topology set where:
    //          T_last_addr == originator address AND
    //          T_seq       <   ANSN
    //  MUST be removed from the topology set.
    #if verbose
    std::cout << "RoutingProtocol::handleTCMessage: Clean all older topology tuple set" << std::endl;
    #endif
    mMtxTopology.lock();
    mState.cleanOlderTopologyTuples(lastAddr, ansn);
    mMtxTopology.unlock();
    //  4.  For each of the advertised neighbor main address received in the TC message:
    //  Process the advertised neighbors
    #if verbose
    std::cout << "RoutingProtocol::handleTCMessage: Process the advertised neighbors" << std::endl;
    #endif
    for (std::vector<MACAddress>::const_iterator it = message.mNeighborAddresses.begin(); it != message.mNeighborAddresses.end(); it++) {
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
        const MACAddress &addr = *it;
        mMtxTopology.lock();
        vTopologyTuple = mState.findTopologyTuple(senderHWAddr, lastAddr);
        mMtxTopology.unlock();
        if (vTopologyTuple != NULL) {
            #if verbose
            std::cout << "RoutingProtocol::handleTCMessage: This tuple is already exist in the topology set" << std::endl;
            #endif
            vTopologyTuple->expirationTime = now + pt::seconds(T_TOP_HOLD_TIME);
        } else {
            #if verbose
            std::cout << "RoutingProtocol::handleTCMessage: A new tuple is recorded in the topology set" << std::endl;
            #endif
            TopologyTuple topologyTuple;
            topologyTuple.destAddr = addr;
            topologyTuple.lastAddr = lastAddr;
            topologyTuple.sequenceNumber = ansn;
            topologyTuple.expirationTime = now + pt::seconds(T_TOP_HOLD_TIME);
            mMtxTopology.lock();
            mState.insertTopologyTuple(topologyTuple);
            mMtxTopology.unlock();
            pt::time_duration expireTimer = topologyTuple.expirationTime - now;
            int seconds = expireTimer.total_seconds();
            boost::thread vExpiryThread = boost::thread(boost::bind(&RoutingProtocol::expireTopology, this, seconds, addr, lastAddr));
        }
    }
}

void RoutingProtocol::updateLinkTuple(LinkTuple * vLinkEdge, uint8_t willingness) {
    #if verbose
    std::cout << "RoutingProtocol::updateLinkTuple: Update the link with neighbor" << std::endl;
    #endif
    pt::ptime now = pt::second_clock::local_time();
    mMtxNeighbor.lock();
    NeighborTuple* vNeighbor = mState.findNeighborTuple(vLinkEdge->neighborIfaceAddr);
    mMtxNeighbor.unlock();
    if (vNeighbor == NULL) {
        // Add advertised neighbor
        NeighborTuple vNewNeighbor;
        vNewNeighbor.neighborMainAddr = vLinkEdge->neighborIfaceAddr;
        vNewNeighbor.willingness = W_WILL_ALWAYS;
        if (vLinkEdge->symTime >= now) {
            vNewNeighbor.status = NeighborTuple::STATUS_SYM;
            #if verbose
            std::cout << "RoutingProtocol::updateState determined neighbor is symetric bidirecitonal" << std::endl;
            #endif
        } else {
            vNewNeighbor.status = NeighborTuple::STATUS_NOT_SYM;
            #if verbose
            std::cout << "RoutingProtocol::updateState determined neighbor is asymetric direcitonal" << std::endl;
            #endif
        }
        #if verbose
        std::cout << "RoutingProtocol::updateLinkTuple: insert new neighbor found" << std::endl;
        #endif
        mMtxNeighbor.lock();
        mState.insertNeighborTuple(vNewNeighbor);
        mMtxNeighbor.unlock();
        vNeighbor = &vNewNeighbor;
        // Increment advertised neighbor set sequence number
        mANSN = (mANSN + 1) % (S_MAX_SEQ_NUM + 1);
        #if verbose
        PRINTLN(RoutingProtocol::updateLinkTuple: Inserted a new neighbor)
        #endif
    }
    // Reset the symTime for this link me -> neighbor
    if (vLinkEdge->symTime >= now) {
        vNeighbor->status = NeighborTuple::STATUS_SYM;
        #if verbose
        std::cout << "RoutingProtocol::updateLinkTuple: Reset sym time to sym for neighbor" << std::endl;
        #endif
    } else {
        vNeighbor->status = NeighborTuple::STATUS_NOT_SYM;
        #if verbose
        std::cout << "RoutingProtocol::updateLinkTuple: Reset sym time to not sym for neighbor" << std::endl;
        #endif
    }

}

void RoutingProtocol::updateMPRState() {
    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Update the MPR to the OLSR state" << std::endl;
    #endif
    // Based on RFC 3626 8.3.1.  MPR Computation
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
    std::set<MACAddress> vMPRs;
    //        N:
    //               N is the subset of neighbors of the node, which are
    //               neighbor of the interface I.
    mMtxNeighbor.lock();
    std::vector<NeighborTuple> N = mState.getNeighbors();
    mMtxNeighbor.unlock();
    //        N2:
    //               The set of 2-hop neighbors reachable from the interface
    //               I, excluding:
    //                (i)   the nodes only reachable by members of N with
    //                      willingness WILL_NEVER
    //                (ii)  the node performing the computation
    //                (iii) all the symmetric neighbors: the nodes for which
    //                      there exists a symmetric link to this node on some
    //                      interface.

    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Compute the MPR base on the neighbor and 2-hop neighbors and MPR set" << std::endl;
    #endif
    mMtxTwoHopNeighbor.lock();
    std::vector<TwoHopNeighborTuple> vCurrentTwoHopNeighbors = mState.getTwoHopNeighbors();
    mMtxTwoHopNeighbor.unlock();
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
        if (filter2hop) {
            N2.push_back(*it);
        }
    }

    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Start with an MPR set made of all members of N with N_willingness and WILL_ALWAYS" << std::endl;
    std::cout << "RoutingProtocol::updateMPRState: Our N2 set is " << N2.size() << std::endl;
    #endif
    //    The proposed MPR heuristic is as follows:
    //      1    Start with an MPR set made of all members of N with
    //           N_willingness equal to WILL_ALWAYS
    for (std::vector<NeighborTuple>::iterator it = N.begin(); it != N.end(); it++) {
        if (it->willingness == W_WILL_ALWAYS) {
            vMPRs.insert(it->neighborMainAddr);
            // Remove the two hop neightbors reachable by MPR from nodes in N2
            removeCoveredTwoHopNeighbor(it->neighborMainAddr, N2);
        }
    }

    //      2    Calculate D(y), where y is a member of N, for all nodes in N.
    //      3    Add to the MPR set those nodes in N, which are the *only*
    //           nodes to provide reachability to a node in N2.  For example,
    //           if node b in N2 can be reached only through a symmetric link
    //           to node a in N, then add node a to the MPR set.  Remove the
    //           nodes from N2 which are now covered by a node in the MPR set.
    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Add to the MPR set those nodes in N, which could reach N2" << std::endl;
    #endif
    std::set<MACAddress> vCoveredTwoHopNeighbors;
    for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
            twoHopNeigh != N2.end(); twoHopNeigh++) {
        bool reachableThruOneIntermediary = true;
        for (std::vector<TwoHopNeighborTuple>::iterator otherTwoHopNeigh = N2.begin();
                otherTwoHopNeigh != N2.end(); otherTwoHopNeigh++) {
            if (twoHopNeigh->twoHopNeighborAddr == otherTwoHopNeigh->twoHopNeighborAddr &&
                    twoHopNeigh->neighborMainAddr != otherTwoHopNeigh->neighborMainAddr) {
                reachableThruOneIntermediary = false;
            }
        }
        if (reachableThruOneIntermediary) {
            #if verbose
            PRINTLN(RoutingProtocol::updateMPRState: Added a neighbor of N2 as MPR)
            #endif
            vMPRs.insert(twoHopNeigh->neighborMainAddr);
            // Find all two hop neightbors of the MPR so we can remove them later
            // because they are already covered by this MPR
            for (std::vector<TwoHopNeighborTuple>::iterator otherNeighs = N2.begin();
                    otherNeighs != N2.end(); otherNeighs++) {
                if (otherNeighs->neighborMainAddr == twoHopNeigh->neighborMainAddr) {
                    vCoveredTwoHopNeighbors.insert(otherNeighs->neighborMainAddr);
                }
            }
        }
    }

    // Perform the removal of the covered neighbors
    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Perform the removal of the coverd Neighbors" << std::endl;
    #endif
    for (std::vector<TwoHopNeighborTuple>::iterator it = N2.begin(); it != N2.end(); ) {
        if (vCoveredTwoHopNeighbors.find(it->twoHopNeighborAddr) != vCoveredTwoHopNeighbors.end()) {
            N2.erase(it);
            #if verbose
            PRINTLN(RoutingProtocol::updateMPRState: Removed a 2 hop neighbor that was already covered by the mpr we just selected)
            #endif
        } else {
            it++;
        }
    }

    //      4    While there exist nodes in N2 which are not covered by at
    //           least one node in the MPR set:
    #if verbose
    std::cout << "RoutingProtocol::updateMPRState: Check the exist nodes in N2 which are not covered in the last one in MPR set" << std::endl;
    #endif
    while (N2.begin() != N2.end()) {
        //           4.1  For each node in N, calculate the reachability, i.e., the
        //                number of nodes in N2 which are not yet covered by at
        //                least one node in the MPR set, and which are reachable
        //                through this 1-hop neighbor;
        std::map<int, std::vector<NeighborTuple*>> vNeighborRechability;
        std::set<int> vNeighborRechabilityKeys;
        for (std::vector<NeighborTuple>::iterator it = N.begin();
                it != N.end(); it++) {
            int key = 0;
            for (std::vector<TwoHopNeighborTuple>::iterator it2hop = N2.begin();
                    it2hop != N2.end(); it2hop++) {
                if (it->neighborMainAddr == it2hop->neighborMainAddr) {
                    key++;
                    #if verbose
                    PRINTLN(RoutingProtocol::updateMPRState: Found a reachable 2 hop neighbor when calculate its reachability)
                    #endif
                }
            }
            vNeighborRechabilityKeys.insert(key);
            vNeighborRechability[key].push_back(&(*it));
        }
        //           4.2  Select as a MPR the node with highest N_willingness among
        //                the nodes in N with non-zero reachability.  In case of
        //                multiple choice select the node which provides
        //                reachability to the maximum number of nodes in N2.  In
        //                case of multiple nodes providing the same amount of
        //                reachability, select the node as MPR whose D(y) is
        //                greater.  Remove the nodes from N2 which are now covered
        //                by a node in the MPR set.
        NeighborTuple *vMaxRechabilityNeighbor = NULL;
        int vMaxRechabilityKey = 0;
        for (std::set<int>::iterator it = vNeighborRechabilityKeys.begin();
                it != vNeighborRechabilityKeys.end (); it++) {
            int key = *it;
            if (key == 0) {
                continue;
            }
            for (std::vector<NeighborTuple*>::iterator it2 = vNeighborRechability[key].begin ();
                    it2 != vNeighborRechability[key].end (); it2++) {
                NeighborTuple *vNeighbor = *it2;
                if (vMaxRechabilityNeighbor == NULL || vNeighbor->willingness > vMaxRechabilityNeighbor->willingness) {
                    vMaxRechabilityNeighbor = vNeighbor;
                    vMaxRechabilityKey = key;
                }
                else if (vNeighbor->willingness == vMaxRechabilityNeighbor->willingness) {
                    if (key > vMaxRechabilityKey) {
                        vMaxRechabilityNeighbor = vNeighbor;
                        vMaxRechabilityKey = key;
                    }
                    else if (key == vMaxRechabilityKey) {
                        if (calculateNodeDegree(*vNeighbor) > calculateNodeDegree (*vMaxRechabilityNeighbor)) {
                            vMaxRechabilityNeighbor = vNeighbor;
                            vMaxRechabilityKey = key;
                        }
                    }
                }
            }
        }

        if (vMaxRechabilityNeighbor != NULL) {
            vMPRs.insert (vMaxRechabilityNeighbor->neighborMainAddr);
            for (std::vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin ();
                    twoHopNeigh != N2.end (); ) {
                if (twoHopNeigh->neighborMainAddr == vMaxRechabilityNeighbor->neighborMainAddr) {
                    #if verbose
                    std::cout << "RoutingProtocol::updateMPRState removed a N2 neightbor caused by non-reachbility" << std::endl;
                    #endif
                    twoHopNeigh = N2.erase (twoHopNeigh);
                }
                else {
                    twoHopNeigh++;
                }
            }
        }
    }
    #if verbose
    PRINTLN(RoutingProtocol::updateMPRState: Finish selecting MPR for reachable 2 hop neighbors)
    #endif
        //      5    A node's MPR set is generated from the union of the MPR sets
        //           for each interface.  As an optimization, process each node, y,
        //           in the MPR set in increasing order of N_willingness.  If all
        //           nodes in N2 are still covered by at least one node in the MPR
        //           set excluding node y, and if N_willingness of node y is
        //           smaller than WILL_ALWAYS, then node y MAY be removed from the
        //           MPR set.
        mMtxMpr.lock();
    mState.setMprSet(vMPRs);
    mMtxMpr.unlock();
}

void RoutingProtocol::removeCoveredTwoHopNeighbor(MACAddress addr, std::vector<TwoHopNeighborTuple>& twoHopNeighbors) {
    // This function will remove all the two hop neighbors that have been covered by an MPR
    #if verbose
    std::cout << "RoutingProtocol::removeCoveredTwoHopNeighbor: Remove all the two hop neighbors that have been covered by an MPR of address " << addr << std::endl;
    #endif
    std::set<MACAddress> uniqueRemovals;
    for (std::vector<TwoHopNeighborTuple>::iterator it = twoHopNeighbors.begin();
            it != twoHopNeighbors.end(); it++) {
        if (it->neighborMainAddr == addr) {
            uniqueRemovals.insert(it->neighborMainAddr);
        }
    }
    for (std::vector<TwoHopNeighborTuple>::iterator it = twoHopNeighbors.begin();
            it != twoHopNeighbors.end();) {
        if (uniqueRemovals.find(it->twoHopNeighborAddr) != uniqueRemovals.end()) {
            #if verbose
            PRINTLN(RoutingProtocol::removeCoveredTwoHopNeighbor: Found and removed a two hop neighbor that has been already covered)
            #endif
            twoHopNeighbors.erase(it);
        } else {
            it++;
        }
    }
}

void RoutingProtocol::setPersonalAddress(const MACAddress & address) {
    #if verbose
    std::cout << "RoutingProtocol::setPersonalAddress: Create OLSR Routing Protocol" << std::endl;
    #endif
    mPersonalAddress.setAddressData(address.data);
}

void RoutingProtocol::routingTableComputation () {
    //  Routing Table Calculation reference to RFC 3626
    //  Procedure of calculate the routing table
    #if verbose
    std::cout << "RoutingProtocol::routingTableComputation: Start calculating the routing table" << std::endl;
    #endif
    pt::ptime now = pt::second_clock::local_time();

    //  1.  All the entries from the routing table are removed.
    #if verbose
    std::cout << "RoutingProtocol::routingTableComputation: Clean all entries of the routing table" << std::endl;
    #endif
    mTable.clear();

    //  2.  The new routing entries are added starting with the symmetric neighbors (h=1) as the destination nodes.
    mMtxNeighbor.lock();
    std::vector<NeighborTuple> &neighborSet = mState.getNeighbors();
    mMtxNeighbor.unlock();
    #if verbose
    std::cout << "RoutingProtocol::routingTableComputation: Check the neighbors if it starting with the symmetric neighbors" << std::endl;
    #endif
    for (std::vector<NeighborTuple>::const_iterator it = neighborSet.begin(); it != neighborSet.end(); it++)
    {
        //  Thus, for each neighbor tuple in the neighbor set where:
        //              N_status        = SYM
        const NeighborTuple &nbTuple = *it;
        if (nbTuple.status == NeighborTuple::STATUS_SYM) {

            //  (There is a symmetric link to the neighbor), and for each associated link tuple of the neighbor node such that
            mMtxLink.lock();
            std::vector<LinkTuple> &linkTupleSet = mState.getLinks();
            mMtxLink.unlock();
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
                    MACAddress dest = linkTuple.neighborIfaceAddr;
                    MACAddress next = linkTuple.neighborIfaceAddr;
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
                        #if verbose
                        std::cout << "RoutingProtocol::routingTableComputation: Add the routing entry to the routing table" << std::endl;
                        #endif
                    }
                }
            }
        }
    }

    //  3.  For each node in N2, i.e., a 2-hop neighbor which is not a neighbor node or the node itself, and such that there
    #if verbose
    std::cout << "RoutingProtocol::routingTableComputation: Check the two hop neighbors if it is not a neighbor or itself" << std::endl;
    #endif
    mMtxTwoHopNeighbor.lock();
    const std::vector<TwoHopNeighborTuple> &twoHopNeighbors = mState.getTwoHopNeighbors();
    mMtxTwoHopNeighbor.unlock();
    for (std::vector<TwoHopNeighborTuple>::const_iterator twoHopNb_it = twoHopNeighbors.begin(); twoHopNb_it != twoHopNeighbors.end(); twoHopNb_it++)
    {
        const TwoHopNeighborTuple &neighbor2HopTuple = *twoHopNb_it;
        mMtxNeighbor.lock();
        bool found = mState.findNeighborTuple(neighbor2HopTuple.twoHopNeighborAddr);
        mMtxNeighbor.unlock();
        if (!found || neighbor2HopTuple.twoHopNeighborAddr != mPersonalAddress) {
            //  exist at least one entry in the 2-hop neighbor set where N_neighbor_main_addr correspond to a neighbor node with
            //  willingness different of WILL_NEVER, one selects one 2-hop tuple and creates one entry in the routing table with:
            bool nb2hopOk = false;
            mMtxNeighbor.lock();
            const std::vector<NeighborTuple> &neighborSet = mState.getNeighbors();
            mMtxNeighbor.unlock();
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
                MACAddress destAddr = neighbor2HopTuple.neighborMainAddr;
                MACAddress nextAddr = neighbor2HopTuple.twoHopNeighborAddr;
                std::map<MACAddress, RoutingTableEntry>::const_iterator entry_it = mTable.find(destAddr);
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
                    #if verbose
                    std::cout << "RoutingProtocol::routingTableComputation: Add the routing entry to the routing table" << std::endl;
                    #endif
                }
            }
        }
    }

#if verbose
    std::cout << "RoutingProtocol::routingTableComputation: Check the destination nodes starting with h = 2 and incrementing it by 1 each time" << std::endl;
    #endif
    for (uint32_t h = 2; ; h++)
    {
        //  The new route entries for the destionation nodes h+1 hops away are recoreded in the routing table. The following procedure
        //  MUST be executed for each value of h, starting with h=2 and incrementing it by 1 each time. The execution will stop if
        //  no new entry is recorded in an iteration.
        bool added = false;
        mMtxTopology.lock();
        const std::vector<TopologyTuple> &topology = mState.getTopologySet();
        mMtxTopology.unlock();
        for (std::vector<TopologyTuple>::const_iterator it = topology.begin(); it != topology.end(); it++)
        {
            const TopologyTuple &topologyTuple = *it;
            //      3.1 For each topology entry in the topology table, if its T_dest_addr does not correspond to R_dest_addr of any
            //      route entry in th routing table AND its T_last_addr corresponds to R_dest_addr of a route entry whose R_dist is
            //      equal to h, then a new route entry MUST be recoreded in the routing table (if it does not already  exist) where:
            RoutingTableEntry destAddrEntry, lastAddrEntry;
            MACAddress destAddr = topologyTuple.destAddr;
            MACAddress lastAddr = topologyTuple.lastAddr;
            std::map<MACAddress, RoutingTableEntry>::const_iterator entry_it = mTable.find(destAddr);
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
                    #if verbose
                    std::cout << "RoutingProtocol::routingTableComputation: Add the routing entry to the routing table" << std::endl;
                    #endif
                }
            }
        }

        //      3.2 Serveral topology entries may be used to select a next hop R_next_addr for reaching the node R_dest_addr.
        //      When h=1, ties should be broken such that nodes with highest willingness and MPR selectors are preferred as next hop.
        if (!added) {
            break;
        }

    }
    #if verbose
    PRINTLN(Done computing routing table)
    #endif
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

int RoutingProtocol::calculateNodeDegree(NeighborTuple & tuple)
{
    #if verbose
    std::cout << "RoutingProtocol::calculateNodeDegree: Calculate the degree of the node" << std::endl;
    #endif
    int degree = 0;
    mMtxTwoHopNeighbor.lock();
    std::vector<TwoHopNeighborTuple> vTwoHopNeightbors = mState.getTwoHopNeighbors();
    mMtxTwoHopNeighbor.unlock();
    for (std::vector<TwoHopNeighborTuple>::const_iterator it =  vTwoHopNeightbors.begin ();
            it != vTwoHopNeightbors.end (); it++) {
        TwoHopNeighborTuple const &vTwoHopNeighrborTuple = *it;
        if (vTwoHopNeighrborTuple.neighborMainAddr == tuple.neighborMainAddr) {
            mMtxNeighbor.lock();
            NeighborTuple *vNeighbor = mState.findNeighborTuple(vTwoHopNeighrborTuple.neighborMainAddr);
            mMtxNeighbor.unlock();
            if (vNeighbor == NULL) {
                degree++;
            }
        }
    }
    return degree;
}


void RoutingProtocol::expireTopology(int seconds, MACAddress destAddr, MACAddress lastAddr) {
    #if verbose
    std::cout << "RoutingProtocol::expireTopology TC expire timer started with " << seconds << " seconds" << std::endl;
    #endif

    do {
        sleep(seconds);
        pt::ptime now = pt::second_clock::local_time();

        mMtxTopology.lock();
        TopologyTuple* vTopologyTuple = mState.findTopologyTuple(destAddr, lastAddr);
        mMtxTopology.unlock();
        if (vTopologyTuple == NULL) {
            #if verbose
            std::cout << "RoutingProtocol::expireTopology TC lost contact with a node " << vTopologyTuple->destAddr << " from neighbor "
                      << vTopologyTuple->lastAddr << std::endl;
             #endif
            return;
        }
        pt::time_duration expireTimer = vTopologyTuple->expirationTime - now;
        seconds = expireTimer.total_seconds();
        if (vTopologyTuple->expirationTime < now) {
            mMtxTopology.lock();
            mState.cleanTopologyTuple(*vTopologyTuple);
            mMtxTopology.unlock();
            #if verbose
            std::cout << "RoutingProtocol::expireTopology TC lost contact with node " << vTopologyTuple->destAddr << " from neighbor "
                      << vTopologyTuple->lastAddr << std::endl;
            #endif
            return;
        }
        #if verbose
        std::cout << "RoutingProtocol::expireTopology TC has a live link with node " << vTopologyTuple->destAddr << " from neighbor "
                  << vTopologyTuple->lastAddr << std::endl;
        #endif
    } while (seconds > 0);
}
