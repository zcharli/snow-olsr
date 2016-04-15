# Get the interface name using "ifconfig", e.g., wlx1cbdb9883233, and
# update the script.
# For help, see "man iwconfig".
# put the device down
ifconfig wlx1cbdb9883233 down
# none cell, no access point
iwconfig wlx1cbdb9883233 mode Ad-hoc
# cell name
iwconfig wlx1cbdb9883233 essid SnowNetwork
# channel
iwconfig wlx1cbdb9883233 channel 3
# data rate
# iwconfig wlx1cbdb9883233 rate 11MB
# identity of ad hoc network cell
iwconfig wlx1cbdb9883233 ap 00:00:00:00:00:01
# put the device up
ifconfig wlx1cbdb9883233 up
iwconfig wlx1cbdb9883233

./Snow-OLSR
