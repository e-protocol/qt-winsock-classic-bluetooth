# qt-winsock-classic-bluetooth

This is example of how to use winsock bluetooth in Qt. 
It has minimum requiered functions to work with classic bluetooth under Windows 10.
It's interfaces are similar to QBluetooth.
Can be compiled under MinGW. For that add this to your .pro file:
LIBS += -lws2_32 -lwsock32 lbthprops
