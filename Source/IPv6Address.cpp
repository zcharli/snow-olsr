#include "Headers/IPv6Address.h"

IPv6Address::IPv6Address() {}
IPv6Address::IPv6Address(char* address) {
    memcpy ( data, address, WLAN_ADDR_LEN );
}
IPv6Address::IPv6Address(const IPv6Address& addr) {
    memcpy ( data, addr.data, WLAN_ADDR_LEN );
}
IPv6Address::~IPv6Address() {}

// Return the address in a human readable form, cred. Michel Barbeau
char* IPv6Address::wlan2asc(char str[]) {
    sprintf(str, "%x:%x:%x:%x:%x:%x",
            data[0], data[1], data[2], data[3], data[4], data[5]);
    return str;
}

char* IPv6Address::wlan2asc(char str[]) const {
    sprintf(str, "%x:%x:%x:%x:%x:%x",
            data[0], data[1], data[2], data[3], data[4], data[5]);
    return str;
}


// Return the address in a human readable form, cred. Michel Barbeau
int IPv6Address::str2wlan(char s[]) {
    int a[6], i;
    // parse the address
    if (sscanf6(s, a, a + 1, a + 2, a + 3, a + 4, a + 5) < 6) {
        return -1;
    }
    // make sure the value of every component does not exceed on byte
    for (i = 0; i < 6; i++) {
        if (a[i] > 0xff) return -1;
    }
    // assign the result to the member "data"
    for (i = 0; i < 6; i++) {
        data[i] = a[i];
    }
    return 0;
}

// convert an address string to a series of hex digits,  cred. Michel Barbeau
int IPv6Address::sscanf6(char str[], int *a1, int *a2, int *a3, int *a4, int *a5, int *a6) {
    int n;
    *a1 = *a2 = *a3 = *a4 = *a5 = *a6 = 0;
    while ((n = hexdigit(*str)) >= 0)
        (*a1 = 16 * (*a1) + n, str++);
    if (*str++ != ':') return 1;
    while ((n = hexdigit(*str)) >= 0)
        (*a2 = 16 * (*a2) + n, str++);
    if (*str++ != ':') return 2;
    while ((n = hexdigit(*str)) >= 0)
        (*a3 = 16 * (*a3) + n, str++);
    if (*str++ != ':') return 3;
    while ((n = hexdigit(*str)) >= 0)
        (*a4 = 16 * (*a4) + n, str++);
    if (*str++ != ':') return 4;
    while ((n = hexdigit(*str)) >= 0)
        (*a5 = 16 * (*a5) + n, str++);
    if (*str++ != ':') return 5;
    while ((n = hexdigit(*str)) >= 0)
        (*a6 = 16 * (*a6) + n, str++);
    return 6;
}

// Convert a char to a hex digit, cred Michel Barbeau
int IPv6Address::hexdigit(char a) {
    if (a >= '0' && a <= '9') return (a - '0');
    if (a >= 'a' && a <= 'f') return (a - 'a' + 10);
    if (a >= 'A' && a <= 'F') return (a - 'A' + 10);
    return -1;
}

void IPv6Address::setAddressData(const char addr[]) {
    memcpy ( data, addr, WLAN_ADDR_LEN );
}
