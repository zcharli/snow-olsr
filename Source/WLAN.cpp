// File: WLAN.cpp
// Author: Michel Barbeau
// Version: January 16, 2016

#include "Headers/WLAN.h"

WLAN::~WLAN() {}
// Constructor
WLAN::WLAN(std::string interface) {
    // Set device name
    device = new char[interface.length() + 1];
    device[interface.length()] = 0;
    memcpy(device, interface.c_str(), interface.length());
}

// Set message handler
void WLAN::setHandler(Handler* aHandler) {
    this->aHandler = aHandler;
}

bool WLAN::createSocket() {
    // Create device level socket
    // - AF_PACKET : packet interface on device level
    // - SOCK_RAW : raw packets including link level header
    // create the socket
    if ((ifconfig.sockid = socket(AF_PACKET, SOCK_RAW, 0)) == -1) {
        std::cerr << "Cannot open socket: " << strerror(errno) << "\n";
        return false;
    }
    std::cerr << "socket created!\n";
    return true;
}

bool WLAN::fetchInterfaceIndex() {
    // fetch the interface index
    struct ifreq ifr;
    strcpy(ifr.ifr_name, device);
    if (ioctl(ifconfig.sockid, SIOGIFINDEX, &ifr) < 0) {
        std::cerr << "Failed to fetch ifindex: " << strerror(errno) << "\n";
        return false;
    }
    ifconfig.ifindex = ifr.ifr_ifindex;
    std::cerr << "ifindex is: " << ifr.ifr_ifindex << "\n";
    return true;
}

bool WLAN::fetchHardwareAddress() {
    // fetch the hardware address
    struct ifreq ifr;
    if (ioctl(ifconfig.sockid, SIOCGIFHWADDR, &ifr) == -1) {
        std::cerr << "Failed to fetch hardware address: " << strerror(errno) << "\n";
        return false;
    }
    std::cout << "real hardware address " << ifr.ifr_hwaddr.sa_data << std::endl;
    memcpy(&ifconfig.hwaddr.data, &ifr.ifr_hwaddr.sa_data, WLAN_ADDR_LEN);
    char * addr = new char[32];
    std::cerr << "hardware address is: " << ifconfig.hwaddr.wlan2asc(addr) << "\n";
    delete addr;
    return true;
}

bool WLAN::fetchMTU() {
    // fetch the MTU
    struct ifreq ifr;
    if (ioctl(ifconfig.sockid, SIOCGIFMTU, &ifr) == -1) {
        std::cerr << "Failed to get the MTU: " << strerror(errno) << "\n";
        return false;
    }
    ifconfig.mtu = ifr.ifr_mtu;
    std::cerr << "MTU is: " << ifr.ifr_mtu << "\n";
    return true;
}

bool WLAN::bindSocketToInterface() {
    // bind the socket to the interface
    // only traffic from that interface will be received
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifconfig.ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(ifconfig.sockid, (struct sockaddr*)&sll, sizeof(sll)) < 0) {
        std::cerr << "Failed to bind the socket: " << strerror(errno) << "\n";
        return false;
    }
    std::cerr << "socket bind done\n";
    return true;
}

// Init
bool WLAN::init() {
    std::cout << "Initializing network interface  : " << device << "\n";
    // create the socket
    if (!createSocket())
        return false;
    else if (!fetchInterfaceIndex())
        return false;
    else if (!fetchHardwareAddress())
        return false;
    else if (!fetchMTU())
        return false;
    else if (!bindSocketToInterface())
        return false;
    else
        return true;
}

// build a frame header
void WLAN::buildHeader(char address[], MACAddress *daddr) {
    // conversion of  destination address from ASCII to binary
    daddr->str2wlan(address);
    // store the destination address
    memmove(&hdr.destAddr, daddr->data, WLAN_ADDR_LEN);
    // store the source address
    memmove(&hdr.srcAddr, ifconfig.hwaddr.data, WLAN_ADDR_LEN);
    // set the type field
    hdr.type = htons(IP_TYPE);
}

// set the "to address"
void WLAN::setToAddress(MACAddress *daddr, struct sockaddr_ll *to) {
    to->sll_family = AF_PACKET;
    to->sll_ifindex = ifconfig.ifindex;
    memmove(&(to->sll_addr), daddr->data, WLAN_ADDR_LEN);
    to->sll_halen = 6;
}

// Send
bool WLAN::send(char address[], char* msg_buffer, int size) {
    // send buffer
    //char buff[WLAN_HEADER_LEN+strlen(message)];
    // destination address
    MACAddress daddr;
    // build the header
    buildHeader(address, &daddr);
    // store the header into the frame
    memmove(msg_buffer, &hdr, WLAN_HEADER_LEN);
    // encapsulate the message into the frame
    //memmove(buff+WLAN_HEADER_LEN, message, strlen(message));
    // set the "to address"
    struct sockaddr_ll to = {0};
    setToAddress(&daddr, &to);
    // send a frame
    int sentlen = sendto(
                      ifconfig.sockid, msg_buffer, size, 0,
                      (sockaddr *) &to, sizeof(to));
    //std::cout << "Send: " <<sentlen <<" " << daddr << std::endl;
    // Check errors
    if (sentlen == -1 ) {
        std::cerr << "WLAN::sendto() failed: " << strerror(errno) << "\n";
        return false;
    }
    return true;
}

// Parse a received frame
void WLAN::parseReceivedFrame(std::shared_ptr<Packet> inPacket) {
    // casting to the WLAN header format
    WLANHeader * wlanHdr = (WLANHeader *) (inPacket->getBuffer());
    // get gestination in ascii
    char *dst = new char[32];
    wlanHdr->destAddr.wlan2asc(dst);
    inPacket->setDestination(wlanHdr->destAddr);
    // get source in ascii
    char *src = new char[32];
    wlanHdr->srcAddr.wlan2asc(src);
    inPacket->setSource(wlanHdr->srcAddr);
    // get my address in ascii
    char *myaddress = new char[32];
    inPacket->setMyAddress(myaddress);
    inPacket->setOffset(sizeof(WLANHeader));
    // check destination
    //std::cout << "DEBUG: from " << src << " to " << dst << ": " << myaddress<< "\n";
    if (strcmp(dst, ifconfig.hwaddr.wlan2asc(myaddress)) == 0 ||
            dst == WLAN_BROADCAST) {
        // destination address is self or broadcast
        if (aHandler != 0) {
            // call the handler for processing that frame
            //printf("%d\n", strlen(buff));
            //aHandler->handle(src, dst, inPacket->getBuffer() + sizeof(WLANHeader));
        }
    } else {
        std::cout << "Destination is not WLAN broadcast: " << src << std::endl;
    }
}


// Receive
void WLAN::receive(std::shared_ptr<Packet> inPacket) {
    // buffer for received frame
    char * buff = new char[ifconfig.mtu];
    // length of received frame
    int i; // frame length
    // src address of frame
    struct sockaddr_ll from;
    socklen_t fromlen = sizeof(struct sockaddr_ll);

    // loop until a non-empty frame is received
    while (true) {
        // clear buffer
        memset(buff, 0, ifconfig.mtu);
        // wait and receive a frame
        fromlen = sizeof(from);
        i = recvfrom(ifconfig.sockid, buff, ifconfig.mtu, 0,
                     (struct sockaddr *) &from, &fromlen);

        // WLANHeader * wlanHdr = (WLANHeader *) (buff);
        // // get source in ascii
        // char *src = new char[32];
        // wlanHdr->srcAddr.wlan2asc(src);
        // char tw[] = "1c:bd:b9:7e:b5:d4";
        // if(strcmp(src,tw) == 0 || i == -1){
        if (i == -1) { // error
            std::cerr << "Cannot receive data: " << strerror(errno) << "\n";
            // sleep for 10 milliseconds and try again
            usleep(10000);
        } else { // nor error
            break; // exit the loop
        }
    }
    inPacket->setBuffer(buff);
    // parse a received frame
    parseReceivedFrame(inPacket);
}

const MACAddress& WLAN::getPersonalAddress() const {
    return ifconfig.hwaddr;
}
