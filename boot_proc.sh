# Get the interface name using "ifconfig", e.g., wlx1cbdb97eb663, and
# update the script.
# For help, see "man iwconfig".
# put the device down
ifconfig wlx1cbdb97eb663 down
# none cell, no access point
iwconfig wlx1cbdb97eb663 mode Ad-hoc
# cell name
iwconfig wlx1cbdb97eb663 essid SnowNetwork
# channel
iwconfig wlx1cbdb97eb663 channel 3
# data rate
# iwconfig wlx1cbdb97eb663 rate 11MB
# identity of ad hoc network cell
iwconfig wlx1cbdb97eb663 ap 00:00:00:00:00:01
# put the device up
ifconfig wlx1cbdb97eb663 up
iwconfig wlx1cbdb97eb663

./SnowNetwork
