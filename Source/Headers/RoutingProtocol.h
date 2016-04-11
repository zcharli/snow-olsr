#ifndef ROUTINGPROTOCOL_H
#define ROUTINGPROTOCOL_H

#include <vector>
#include <map>
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
#include "IPv6Address.h"
#include "OLSRState.h"
#include "OLSRMessage.h"
#include "IPv6Address.h"

//using namespace std;

namespace pt = boost::posix_time;

struct RoutingTableEntry
{
    IPv6Address destAddr;
    IPv6Address nextAddr;
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

    // routing table computation should be done after handle TC message
    void routingTableComputation ();

    void setPersonalAddress(const IPv6Address& addr);
    IPv6Address getPersonalAddress(IPv6Address addr) const;
    //void SetMainInterface (uint32_t interface);

private:
    RoutingProtocol () {};
    /// Internal state with all needed data structs.
    std::map<IPv6Address, RoutingTableEntry> mTable;
    OLSRState mState;
    HelloMessage mHelloStateRep;
    TCMessage mTCStateRep;
    IPv6Address mPersonalAddress;
    boost::mutex mMtxState, mMtxLinkExpire, mMtxUpdateLinkTuple, mMtxRoutingTableCalc,
                 mMtxMprUpdate;


    /* Below is protocol controlling functions */
    void handleHelloMessage(HelloMessage&, const IPv6Address&, unsigned char);
    void handleTCMessage(TCMessage&, IPv6Address&);
    void handleTCMessage(std::shared_ptr<OLSRMessage> message);


    void buildHelloMessage();
    void buildTCMessage();

    void updateMPRState();
    void populateTwoHopNeighborSet (const std::shared_ptr<OLSRMessage> &message, const HelloMessage &hello);

    void updateLinkTuple(LinkTuple*, uint8_t);

    /* Below if our timers for updating the state */
    void expireLink(const boost::system::error_code&, boost::asio::deadline_timer*, boost::asio::io_service*, IPv6Address&);


    /* Below is important OLSR attributes */
    // advertised neighbor set sequence number
    uint16_t mANSN;

    // // Entry
    // vector<RoutingTableEntry> GetEntries () const;
    // bool Lookup (const IPv6Address &dest, RoutingTableEntry &outEntry) const;
    // bool FindSendEntry (const RoutingTableEntry &entry, RoutingTableEntry &outEntry) const;
    // void AddEntry (const IPv6Address &dest, const IPv6Address &next, uint32_t interface, uint32_t distance);
    // void RemoveEntry (const IPv6Address &dest);

    // // Routing Table and MPR computation
    // void MprComputation ();
    // void RoutingTableComputation ();

    // // Populate Routing Table
    // void LinkSensing (const TCMessage &msg, const HelloMessage &hello, const IPv6Address &receiverIface, const IPv6Address &sender_iface);
    // void PopulateNeighborSet (const TCMessage &msg, const HelloMessage &hello);
    // void PopulateTwoHopNeighborSet (const TCMessage &msg, const HelloMessage &hello);
    // void PopulateMprSelectorSet (const TCMessage &msg, const HelloMessage &hello);
};

#endif
