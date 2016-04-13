#ifndef ROUTINGPROTOCOL_H
#define ROUTINGPROTOCOL_H

#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <stdint.h>
#include <string>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Resources/Types.h"
#include "Resources/Constants.h"
#include "Resources/Helpers.h"
#include "HelloMessage.h"
#include "TCMessage.h"
#include "MACAddress.h"
#include "OLSRState.h"
#include "OLSRMessage.h"
#include "MACAddress.h"

namespace pt = boost::posix_time;

struct RoutingTableEntry
{
    MACAddress destAddr;
    MACAddress nextAddr;
    uint32_t interface;
    uint32_t distance;

    RoutingTableEntry () :
        destAddr (), nextAddr (),
        interface (0), distance (0) {};
};


class RoutingProtocol
{
public:
    static RoutingProtocol& getInstance() {
        static RoutingProtocol instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    };
    void updateState(std::shared_ptr<OLSRMessage> message);

    // This should be private because when every times hanle message this is be called to update.
    // This need to disccuss
    void mprComputation ();

    // routing table computation should be called after handle TC message
    int buildHelloMessage(OLSRMessage&);
    int buildTCMessage(OLSRMessage&);

    // routing table computation should be done after handle TC message
    void routingTableComputation ();

    void setPersonalAddress(const MACAddress& addr);
    MACAddress getPersonalAddress(MACAddress addr) const;
    //void SetMainInterface (uint32_t interface);
    void lockForUpdate() {
        mMtxSystem.lock();
    }

    void unLockAfterUpdate() {
        mMtxSystem.unlock();
    }

private:
    RoutingProtocol () {
        mHelloSequenceNumber = 0;
        mTCSequenceNumber = 0;
    };
    /// Internal state with all needed data structs.
    std::map<MACAddress, RoutingTableEntry> mTable;
    OLSRState mState;
    HelloMessage mHelloStateRep;
    TCMessage mTCStateRep;
    MACAddress mPersonalAddress;
    boost::mutex mMtxState, mMtxLinkExpire, mMtxUpdateLinkTuple, mMtxRoutingTableCalc,
                 mMtxMprUpdate, mMtxDegree, mMtxGetHello, mMtxGetTc, mMtxSystem;

    int mHelloSequenceNumber;
    int mTCSequenceNumber;

    /* Below is protocol controlling functions */
    void handleHelloMessage(HelloMessage&, const MACAddress&, unsigned char);
    void handleTCMessage(TCMessage&, MACAddress&);
    void handleTCMessage(std::shared_ptr<OLSRMessage> message);

    void updateMPRState();
    void removeCoveredTwoHopNeighbor(MACAddress, std::vector<TwoHopNeighborTuple>&);
    void populateTwoHopNeighborSet (const std::shared_ptr<OLSRMessage> &message, const HelloMessage &hello);
    int calculateNodeDegree(NeighborTuple &);

    void updateLinkTuple(LinkTuple*, uint8_t);

    /* Below if our timers for updating the state */
    void expireLink(const boost::system::error_code&, boost::asio::deadline_timer*, boost::asio::io_service*, MACAddress&);


    /* Below is important OLSR attributes */
    // advertised neighbor set sequence number
    uint16_t mANSN;
};

#endif
