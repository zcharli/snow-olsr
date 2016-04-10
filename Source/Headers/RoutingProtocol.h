#ifndef ROUTINGPROTOCOL_H
#define ROUTINGPROTOCOL_H

#include <vector>
#include <map>
#include <stdint.h>
#include <string>
#include <memory>
#include <iostream>
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

	void setPersonalAddress(const IPv6Address& addr);
	IPv6Address getPersonalAddress(IPv6Address addr) const;
	//void SetMainInterface (uint32_t interface);

private:
	RoutingProtocol () {};
	/// Internal state with all needed data structs.
	OLSRState mState;
	HelloMessage mHelloStateRep;
	TCMessage mTCStateRep;
	IPv6Address mPersonalAddress;

	void handleTCMessage(TCMessage&, IPv6Address&);
	void handleHelloMessage(HelloMessage&, const IPv6Address&, unsigned char);
	// void handleHelloMessage(std::shared_ptr<OLSRMessage> message);
	void handleTCMessage(std::shared_ptr<OLSRMessage> message);


	void buildHelloMessage();
	void buildTCMessage();

	void updateMPRState();
	void populateTwoHopNeighborSet (const std::shared_ptr<OLSRMessage> &message, const HelloMessage &hello);

    boost::mutex mMtxState;


	// IPv6Address m_mainAddress;
	// IPv6Address m_routingAgentAddr;
	// uint16_t m_ansn;
	// uint8_t m_willingness;

	// map< Ptr<Socket>, IPv6Address > m_socketAddresses;
	// map<IPv6Address, RoutingTableEntry> m_table;

	// void SetIpv6(Ptr<Ipv6> ipv6);

	// uint32_t GetSize () const { return m_table.size (); }
	// IPv6Address GetMainAddress (IPv6Address iface_addr) const;

	// // Entry
	// vector<RoutingTableEntry> GetEntries () const;
	// bool Lookup (const IPv6Address &dest, RoutingTableEntry &outEntry) const;
	// bool FindSendEntry (const RoutingTableEntry &entry, RoutingTableEntry &outEntry) const;
	// void AddEntry (const IPv6Address &dest, const IPv6Address &next, uint32_t interface, uint32_t distance);
	// void RemoveEntry (const IPv6Address &dest);

	// // Routing Table and MPR computation
	// void MprComputation ();
	// void RoutingTableComputation ();

	// // Hello Message
	// void SendHello ();
	// void ProcessHello (const HelloMessage &msg, const IPv6Address &receiverIface, const IPv6Address &senderIface);

	// // Manipulate Routing Table
	// void NeighborLoss (const LinkTuple &tuple);
	// void AddDuplicateTuple (const DuplicateTuple &tuple);
	// void RemoveDuplicateTuple (const DuplicateTuple &tuple);
	// void LinkTupleAdded (const LinkTuple &tuple, uint8_t willingness);
	// void RemoveLinkTuple (const LinkTuple &tuple);
	// void LinkTupleUpdated (const LinkTuple &tuple, uint8_t willingness);
	// void AddNeighborTuple (const NeighborTuple &tuple);
	// void RemoveNeighborTuple (const NeighborTuple &tuple);
	// void AddTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);
	// void RemoveTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple);
	// void AddMprSelectorTuple (const MprSelectorTuple &tuple);
	// void RemoveMprSelectorTuple (const MprSelectorTuple &tuple);
	// void AddTopologyTuple (const TopologyTuple &tuple);
	// void RemoveTopologyTuple (const TopologyTuple &tuple);
	// void AddIfaceAssocTuple (const InterfaceAssociationTuple &tuple);
	// void RemoveIfaceAssocTuple (const InterfaceAssociationTuple &tuple);

	// // Populate Routing Table
	// void LinkSensing (const TCMessage &msg, const HelloMessage &hello, const IPv6Address &receiverIface, const IPv6Address &sender_iface);
	// void PopulateNeighborSet (const TCMessage &msg, const HelloMessage &hello);
	// void PopulateTwoHopNeighborSet (const TCMessage &msg, const HelloMessage &hello);
	// void PopulateMprSelectorSet (const TCMessage &msg, const HelloMessage &hello);
};

#endif
