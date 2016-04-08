#ifndef I_PV6ADDRESS_H
#define I_PV6ADDRESS_H

#include <iostream>
#include <string.h>
#include "Resources/Constants.h"

using namespace std;

class IPv6Address
{
public:
    IPv6Address();
    ~IPv6Address();
    char* wlan2asc(char str[]);
    int str2wlan(char s[]);
    unsigned char data[WLAN_ADDR_LEN];
    static int sscanf6(char str[], int *a1, int *a2, int *a3, int *a4, int *a5,
        int *a6);
    static int hexdigit(char a);
};

#endif // I_PV6ADDRESS_H
