#pragma once
#ifndef BLUETOOTHCALL_H
#define BLUETOOTHCALL_H

#include <winsock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>

class BluetoothCall
{
public:
    static int callConnect(SOCKET sock, SOCKADDR_BTH addrBth)
    {
        return connect(sock, (struct sockaddr *) &addrBth, sizeof(SOCKADDR_BTH));
    };

    static int callShutdown(SOCKET sock)
    {
        return shutdown(sock, SD_BOTH);
    };

    static int callBind(SOCKET sock, SOCKADDR_BTH addrBth)
    {
        return bind(sock, (struct sockaddr *) &addrBth, sizeof(SOCKADDR_BTH));
    };
};

#endif // BLUETOOTHCALL_H
