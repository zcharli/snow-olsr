# Get the interface name using "ifconfig", e.g., wlxf07d6813a47c, and
# update the script.
# For help, see "man iwconfig".
# put the device down
ifconfig wlxf07d6813a47c down
# none cell, no access point
iwconfig wlxf07d6813a47c mode Ad-hoc
# cell name
iwconfig wlxf07d6813a47c essid SnowNetwork
# channel
iwconfig wlxf07d6813a47c channel 3
# data rate
# iwconfig wlxf07d6813a47c rate 11MB
# identity of ad hoc network cell
iwconfig wlxf07d6813a47c ap 00:00:00:00:00:01
# put the device up
ifconfig wlxf07d6813a47c up
iwconfig wlxf07d6813a47c

