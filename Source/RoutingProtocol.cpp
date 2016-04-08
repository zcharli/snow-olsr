#include "Headers/RoutingProtocol.h"


// RoutingProtocol::~RoutingProtocol()
// {}

// void RoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
// {
// 	m_ansn = S_MAX_SEQ_NUM;
// 	m_linkTupleTimerFirstTime = true;
// 	m_ipv6 = ipv6;
// }

// void RoutingProtocol::SetMainInterface (uint32_t interface)
// {
// 	// m_mainAddress = m_ipv6->GetAddress (interface, 0).GetLocal();
// }

// void RoutingProtocol::MprComputation()
// {
// 	set<IPv6Address> mprSet;
// 	vector<NeighborTuple> N;

// 	for (vector<NeighborTuple>::const_iterator neighbor = m_state.GetNeighbors().begin();
// 	     neighbor != m_state.GetNeighbors().end(); neighbor++)
// 	{
// 		if (neighbor->status == NeighborTuple::STATUS_SYM) // I think that we need this check
// 		{
// 			N.push_back (*neighbor);
// 		}
// 	}

// 	vector<TwoHopNeighborTuple> N2;
// 	for (vector<TwoHopNeighborTuple>::const_iterator twoHopNeigh = m_state.GetTwoHopNeighbors().begin();
// 	     twoHopNeigh != m_state.GetTwoHopNeighbors().end(); twoHopNeigh++)
// 	{
// 		if (twoHopNeigh->twoHopNeighborAddr == m_mainAddress) {
// 			continue;
// 		}

// 		bool ok = false;
// 		for (vector<NeighborTuple>::const_iterator neigh = N.begin();
// 		     neigh != N.end(); neigh++)
// 		{
// 			if (neigh->neighborMainAddr == twoHopNeigh->neighborMainAddr)
// 			{
// 				if (neigh->willingness == OLSR_WILL_NEVER)
// 				{
// 					ok = false;
// 					break;
// 				}
// 				else
// 				{
// 					ok = true;
// 					break;
// 				}
// 			}
// 		}
// 		if (!ok)
// 		{
// 			continue;
// 		}

// 		// excluding:
// 		// (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
// 		//		 link to this node on some interface.
// 		for (vector<NeighborTuple>::const_iterator neigh = N.begin();
// 		     neigh != N.end(); neigh++)
// 		{
// 			if (neigh->neighborMainAddr == twoHopNeigh->twoHopNeighborAddr)
// 			{
// 				ok = false;
// 				break;
// 			}
// 		}

// 		if (ok)
// 		{
// 			N2.push_back (*twoHopNeigh);
// 		}
// 	}

// 	// 1. Start with an MPR set made of all members of N with
// 	// N_willingness equal to WILL_ALWAYS
// 	for (vector<NeighborTuple>::const_iterator neighbor = N.begin(); neighbor != N.end(); neighbor++)
// 	{
// 		if (neighbor->willingness == OLSR_WILL_ALWAYS)
// 		{
// 			mprSet.insert (neighbor->neighborMainAddr);
// 			// (not in RFC but I think is needed: remove the 2-hop
// 			// neighbors reachable by the MPR from N2)
// 			for (vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
// 			     twoHopNeigh != N2.end(); )
// 			{
// 				if (twoHopNeigh->neighborMainAddr == neighbor->neighborMainAddr)
// 				{
// 					twoHopNeigh = N2.erase (twoHopNeigh);
// 				}
// 				else
// 				{
// 					twoHopNeigh++;
// 				}
// 			}
// 		}
// 	}

// 	// 2. Calculate D(y), where y is a member of N, for all nodes in N.
// 	// (we do this later)

// 	// 3. Add to the MPR set those nodes in N, which are the *only*
// 	// nodes to provide reachability to a node in N2.
// 	std::set<Ipv6Address> coveredTwoHopNeighbors;
// 	for (vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin(); twoHopNeigh != N2.end(); twoHopNeigh++)
// 	{
// 		vector<NeighborTuple>::const_iterator onlyNeighbor = N.end();

// 		for (vector<NeighborTuple>::const_iterator neighbor = N.begin();
// 		     neighbor != N.end(); neighbor++)
// 		{
// 			if (neighbor->neighborMainAddr == twoHopNeigh->neighborMainAddr)
// 			{
// 				if (onlyNeighbor == N.end())
// 				{
// 					onlyNeighbor = neighbor;
// 				}
// 				else
// 				{
// 					onlyNeighbor = N.end();
// 					break;
// 				}
// 			}
// 		}
// 		if (onlyNeighbor != N.end())
// 		{
// 			mprSet.insert (onlyNeighbor->neighborMainAddr);
// 			coveredTwoHopNeighbors.insert (twoHopNeigh->twoHopNeighborAddr);
// 		}
// 	}
// 	// Remove the nodes from N2 which are now covered by a node in the MPR set.
// 	for (vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
// 	     twoHopNeigh != N2.end(); )
// 	{
// 		if (coveredTwoHopNeighbors.find (twoHopNeigh->twoHopNeighborAddr) != coveredTwoHopNeighbors.end())
// 		{
// 			twoHopNeigh = N2.erase (twoHopNeigh);
// 		}
// 		else
// 		{
// 			twoHopNeigh++;
// 		}
// 	}

// 	// 4. While there exist nodes in N2 which are not covered by at
// 	// least one node in the MPR set:
// 	while (N2.begin() != N2.end())
// 	{
// 		// 4.1. For each node in N, calculate the reachability, i.e., the
// 		// number of nodes in N2 which are not yet covered by at
// 		// least one node in the MPR set, and which are reachable
// 		// through this 1-hop neighbor
// 		std::map<int, std::vector<const NeighborTuple *> > reachability;
// 		std::set<int> rs;
// 		for (vector<NeighborTuple>::iterator it = N.begin(); it != N.end(); it++)
// 		{
// 			NeighborTuple const &nb_tuple = *it;
// 			int r = 0;
// 			for (vector<TwoHopNeighborTuple>::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
// 			{
// 				TwoHopNeighborTuple const &nb2hop_tuple = *it2;
// 				if (nb_tuple.neighborMainAddr == nb2hop_tuple.neighborMainAddr)
// 					r++;
// 			}
// 			rs.insert (r);
// 			reachability[r].push_back (&nb_tuple);
// 		}

// 		// 4.2. Select as a MPR the node with highest N_willingness among
// 		// the nodes in N with non-zero reachability. In case of
// 		// multiple choice select the node which provides
// 		// reachability to the maximum number of nodes in N2. In
// 		// case of multiple nodes providing the same amount of
// 		// reachability, select the node as MPR whose D(y) is
// 		// greater. Remove the nodes from N2 which are now covered
// 		// by a node in the MPR set.
// 		NeighborTuple const *max = NULL;
// 		int max_r = 0;
// 		for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
// 		{
// 			int r = *it;
// 			if (r == 0)
// 			{
// 				continue;
// 			}
// 			for (std::vector<const NeighborTuple *>::iterator it2 = reachability[r].begin();
// 			     it2 != reachability[r].end(); it2++)
// 			{
// 				const NeighborTuple *nb_tuple = *it2;
// 				if (max == NULL || nb_tuple->willingness > max->willingness)
// 				{
// 					max = nb_tuple;
// 					max_r = r;
// 				}
// 				else if (nb_tuple->willingness == max->willingness)
// 				{
// 					if (r > max_r)
// 					{
// 						max = nb_tuple;
// 						max_r = r;
// 					}
// 					else if (r == max_r)
// 					{
// 						if (Degree (*nb_tuple) > Degree (*max))
// 						{
// 							max = nb_tuple;
// 							max_r = r;
// 						}
// 					}
// 				}
// 			}
// 		}

// 		if (max != NULL)
// 		{
// 			mprSet.insert (max->neighborMainAddr);
// 			for (vector<TwoHopNeighborTuple>::iterator twoHopNeigh = N2.begin();
// 			     twoHopNeigh != N2.end(); )
// 			{
// 				if (twoHopNeigh->neighborMainAddr == max->neighborMainAddr)
// 				{
// 					twoHopNeigh = N2.erase (twoHopNeigh);
// 				}
// 				else
// 				{
// 					twoHopNeigh++;
// 				}
// 			}
// 		}
// 	}
// 	m_state.SetMprSet (mprSet);
// }

// Ipv6Address
// RoutingProtocol::GetMainAddress (Ipv6Address iface_addr) const
// {
// 	const InterfaceAssociationTuple *tuple = m_state.FindIfaceAssocTuple (iface_addr);

// 	if (tuple != NULL)
// 		return tuple->mainAddr;
// 	return iface_addr;
// }

// void RoutingProtocol::RoutingTableComputation()
// {
// 	// 1. All the entries from the routing table are removed.
// 	m_table.clear();

// 	// 2. The new routing entries are added starting with the symmetric neighbors (h=1) as the destination nodes.
// 	const vector<NeighborTuple> &neighborSet = m_state.GetNeighbors();
// 	for (vector<NeighborTuple>::const_iterator it = neighborSet.begin(); it != neighborSet.end(); it++)
// 	{
// 		NeighborTuple const &nb_tuple = *it;
// 		if (nb_tuple.status == NeighborTuple::STATUS_SYM)
// 		{
// 			bool nb_main_addr = false;
// 			const LinkTuple *lt = NULL;
// 			const vector<LinkTuple> &linkSet = m_state.GetLinks();
// 			for (vector<LinkTuple>::const_iterator it2 = linkSet.begin(); it2 != linkSet.end(); it2++)
// 			{
// 				LinkTuple const &link_tuple = *it2;
// 				if ((GetMainAddress (link_tuple.neighborIfaceAddr) == nb_tuple.neighborMainAddr))
// 				{
// 					lt = &link_tuple;
// 					AddEntry (link_tuple.neighborIfaceAddr, link_tuple.neighborIfaceAddr, link_tuple.localIfaceAddr, 1);
// 					if (link_tuple.neighborIfaceAddr == nb_tuple.neighborMainAddr)
// 					{
// 						nb_main_addr = true;
// 					}
// 				}
// 			}

// 			// If, in the above, no R_dest_addr is equal to the main address of the neighbor,
// 			// then another new routing entry with MUST be added, with:
// 			//		R_dest_addr	= main address of the neighbor;
// 			//		R_next_addr	= L_neighbor_iface_addr of one of the associated link tuple with L_time >= current time;
// 			//		R_dist		 = 1;
// 			//		R_iface_addr = L_local_iface_addr of the associated link tuple.
// 			if (!nb_main_addr && lt != NULL)
// 			{
// 				AddEntry(nb_tuple.neighborMainAddr, lt->neighborIfaceAddr, lt->localIfaceAddr, 1);
// 			}
// 		}
// 	}

// 	//	3. for each node in N2, i.e., a 2-hop neighbor which is not a neighbor node or the node itself,
// 	//  and such that there exist at least one entry in the 2-hop neighbor set where
// 	//	N_neighbor_main_addr correspond to a neighbor node with willingness different of WILL_NEVER,
// 	const vector<TwoHopNeighborTuple> &twoHopNeighbors = m_state.GetTwoHopNeighbors();
// 	for (vector<TwoHopNeighborTuple>::const_iterator it = twoHopNeighbors.begin(); it != twoHopNeighbors.end(); it++)
// 	{
// 		TwoHopNeighborTuple const &nb2hop_tuple = *it;
// 		// a 2-hop neighbor which is not a neighbor node or the node itself
// 		if (m_state.FindNeighborTuple(nb2hop_tuple.twoHopNeighborAddr)) { continue; }
// 		if (nb2hop_tuple.twoHopNeighborAddr == m_mainAddress) { continue; }

// 		// ...and such that there exist at least one entry in the 2-hop neighbor set
// 		// where N_neighbor_main_addr correspond to a neighbor node with willingness different of WILL_NEVER...
// 		bool nb2hopOk = false;
// 		for (vector<NeighborTuple>::const_iterator neighbor = neighborSet.begin(); neighbor != neighborSet.end(); neighbor++)
// 		{
// 			if (neighbor->neighborMainAddr == nb2hop_tuple.neighborMainAddr && neighbor->willingness != OLSR_WILL_NEVER)
// 			{
// 				nb2hopOk = true;
// 				break;
// 			}
// 		}
// 		if (!nb2hopOk) { continue; }

// 		// one selects one 2-hop tuple and creates one entry in the routing table with:
// 		//		R_dest_addr	=	the main address of the 2-hop neighbor;
// 		//		R_next_addr	= the R_next_addr of the entry in the routing table with:
// 		//						 R_dest_addr == N_neighbor_main_addr of the 2-hop tuple;
// 		//		R_dist		 = 2;
// 		//		R_iface_addr = the R_iface_addr of the entry in the routing table with:
// 		//						 R_dest_addr == N_neighbor_main_addr of the 2-hop tuple;
// 		RoutingTableEntry entry;
// 		bool foundEntry = Lookup (nb2hop_tuple.neighborMainAddr, entry);

// 		if (foundEntry) {
// 			AddEntry (nb2hop_tuple.twoHopNeighborAddr, entry.nextAddr, entry.interface, 2);
// 		}
// 	}

// 	for (uint32_t h = 2; ; h++)
// 	{
// 		bool added = false;

// 		// 3.1. For each topology entry in the topology table, if its T_dest_addr does not correspond
// 		// to R_dest_addr of any route entry in the routing table AND its T_last_addr corresponds to
// 		// R_dest_addr of a route entry whose R_dist is equal to h, then a new route entry MUST be recorded in
// 		// the routing table (if it does not already exist)
// 		const vector<TopologyTuple> &topology = m_state.GetTopologySet();
// 		for (vector<TopologyTuple>::const_iterator it = topology.begin(); it != topology.end(); it++)
// 		{
// 			const TopologyTuple &topology_tuple = *it;

// 			RoutingTableEntry destAddrEntry, lastAddrEntry;
// 			bool have_destAddrEntry = Lookup (topology_tuple.destAddr, destAddrEntry);
// 			bool have_lastAddrEntry = Lookup (topology_tuple.lastAddr, lastAddrEntry);
// 			if (!have_destAddrEntry && have_lastAddrEntry && lastAddrEntry.distance == h)
// 			{
// 				// then a new route entry MUST be recorded in
// 				// the routing table (if it does not already exist) where:
// 				//		R_dest_addr	= T_dest_addr;
// 				//		R_next_addr	= R_next_addr of the recorded route entry where:
// 				//						R_dest_addr == T_last_addr
// 				//		R_dist		 = h+1;
// 				//	and
// 				//		R_iface_addr = R_iface_addr of the recorded
// 				//	route entry where:
// 				//		R_dest_addr == T_last_addr.
// 				AddEntry (topology_tuple.destAddr, lastAddrEntry.nextAddr, lastAddrEntry.interface, h + 1);
// 				added = true;
// 			}
// 		}

// 		if (!added) break;
// 	}

// 	// 4. For each entry in the multiple interface association base where there exists a routing entry
// 	// such that:								R_dest_addr	== I_main_addr	(of the multiple interface association entry)
// 	// AND there is no routing entry such that:	R_dest_addr	== I_iface_addr
// 	const vector<InterfaceAssociationTuple> &ifaceAssocSet = m_state.GetIfaceAssocSet();
// 	for (vector<InterfaceAssociationTuple>::const_iterator it = ifaceAssocSet.begin(); it != ifaceAssocSet.end(); it++)
// 	{
// 		InterfaceAssociationTuple const &tuple = *it;
// 		RoutingTableEntry entry1, entry2;
// 		bool have_entry1 = Lookup (tuple.mainAddr, entry1);
// 		bool have_entry2 = Lookup (tuple.ifaceAddr, entry2);
// 		if (have_entry1 && !have_entry2)
// 		{
// 			// then a route entry is created in the routing table with:
// 			//		R_dest_addr		=	I_iface_addr (of the multiple interface association entry)
// 			//		R_next_addr		=	R_next_addr	 (of the recorded route entry)
// 			//		R_dist			=	R_dist		 (of the recorded route entry)
// 			//		R_iface_addr	=	R_iface_addr (of the recorded route entry).
// 			AddEntry (tuple.ifaceAddr, entry1.nextAddr, entry1.interface, entry1.distance);
// 		}
// 	}
// }


