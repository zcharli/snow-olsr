# Get the interface name using "ifconfig", e.g., wlx1cbdb97eb66b, and
# update the script.
# For help, see "man iwconfig".
# put the device down
ifconfig wlx1cbdb97eb66b down
# none cell, no access point
iwconfig wlx1cbdb97eb66b mode Ad-hoc
# cell name
iwconfig wlx1cbdb97eb66b essid SnowNetwork
# channel
iwconfig wlx1cbdb97eb66b channel 3
# data rate
# iwconfig wlx1cbdb97eb66b rate 11MB
# identity of ad hoc network cell
iwconfig wlx1cbdb97eb66b ap 00:00:00:00:00:01
# put the device up
ifconfig wlx1cbdb97eb66b up
iwconfig wlx1cbdb97eb66b

