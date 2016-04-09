#ifndef WLAN_H
#define WLAN_H
// File: WLAN.h
// Author: Michel Barbeau
// Version: January 16, 2016

// C Headers
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// C++ Headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "Handler.h"
#include "Packet.h"
#include "IPv6Address.h"
#include "Resources/Constants.h"

using namespace std;

// WLAN Class Definition
class WLAN {
public:
   // Constructor
   WLAN(string interface);
   // Destructor
   ~WLAN();
   // Initialize
   bool init();
   // Send a frame
   bool send(char address[], char message[]);
   // Set a handler
   void setHandler(Handler* aHandler);
   // Receive a frame
   void receive(shared_ptr<Packet>);
   // Get the IfConfig
   const IPv6Address& getPersonalAddress() const;
private:
   // Constants
   static const unsigned short IP_TYPE = 0x3901;
   const string WLAN_BROADCAST = "ff:ff:ff:ff:ff:ff";

   // Structure of a frame header
   struct WLANHeader{
       // destination address
       IPv6Address destAddr;
       // source address
       IPv6Address srcAddr;
       // type
       unsigned short type;
   };

   // Structure of a network interface configuration
   struct Ifconfig{
      // socket descriptor
      int sockid;
      // interface index
      int ifindex;
      // mac address
      IPv6Address hwaddr;
      // maximum transmission unit
      int mtu;
   };
   // Network interface label
   char* device;
   // Frame buffer
   unsigned char* buff;
   // Network interface configuration
   Ifconfig ifconfig;
   // Frame header
   WLANHeader hdr;
   // Handler of frame payload
   Handler* aHandler;
   // Initialization helpers
   bool createSocket();
   bool fetchInterfaceIndex();
   bool fetchHardwareAddress();
   bool fetchMTU();
   bool addPromiscuousMode();
   bool bindSocketToInterface();
   // Send helpers
   void buildHeader(char address[], IPv6Address *daddr);
   void setToAddress(IPv6Address *daddr, struct sockaddr_ll *to);
   // Receive helper
   void parseReceivedFrame(shared_ptr<Packet>);
};

#endif // WLAN_H
