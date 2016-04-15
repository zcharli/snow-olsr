#ifndef I_PV6ADDRESS_H
#define I_PV6ADDRESS_H

#include <iostream>
#include <ostream>
#include <string.h>
#include <stdio.h>
#include "Resources/Constants.h"

class MACAddress
{
public:
  MACAddress();
  MACAddress(char*);
  MACAddress(const MACAddress&);
  ~MACAddress();
  char* wlan2asc(char str[]);
  char* wlan2asc(char str[]) const;
  int str2wlan(char s[]);
  unsigned char data[WLAN_ADDR_LEN];
  static int sscanf6(char str[], int *a1, int *a2, int *a3, int *a4, int *a5,
                     int *a6);
  static int hexdigit(char a);
  void setAddressData(const unsigned char[]);
};

inline std::ostream& operator << (std::ostream& os, MACAddress const& address) {
  char* str = new char[32];
  os << address.wlan2asc(str);
  delete [] str;
  return os;
}


inline bool operator==(const MACAddress& lhs, const MACAddress& rhs)
{
  return memcmp ( lhs.data, rhs.data, WLAN_ADDR_LEN ) == 0;
}

inline bool operator!=(const MACAddress& lhs, const MACAddress& rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(const MACAddress& lhs, const MACAddress& rhs)
{
    return memcmp ( lhs.data, rhs.data, WLAN_ADDR_LEN ) < 0;
}

#endif // I_PV6ADDRESS_H
