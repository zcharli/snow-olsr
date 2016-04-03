# Get the interface name using "ifconfig", e.g., wlan1, and
# update the script.
# For help, see "man iwconfig".
# put the device down
ifconfig wlan1 down
# none cell, no access point
iwconfig wlan1 mode Ad-hoc
# cell name
iwconfig wlan1 essid SnowNetwork
# channel
iwconfig wlan1 channel 3
# data rate
# iwconfig wlan1 rate 11MB
# identity of ad hoc network cell
iwconfig wlan1 ap 00:00:00:00:00:01
# put the device up
ifconfig wlan1 up
iwconfig wlan1
