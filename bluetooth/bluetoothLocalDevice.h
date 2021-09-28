#pragma once
#ifndef BLUETOOTHLOCALDEVICE_H
#define BLUETOOTHLOCALDEVICE_H

#include "bluetoothDevice.h"
#include <QHash>
#include <QObject>
#include <QtConcurrent>
#include <QObject>

class BluetoothLocalDevice : public QObject
{
    Q_OBJECT
public:
    BluetoothLocalDevice();
    bool isCycle = false;
    bool isKill = false;
    bool isShutDown = false;

    enum Error
    {
        NoError = 0,
        WinsockError = 1,
        RemoteAddressError = 2,
        SocketError = 3,
        PairingError = 4
    };

    QHash<QString, BluetoothDevice>* getDevicesList() { return devicesList; }
    bool isValid() const;
    bool enumirateDevices();
    void requestPairing(const QString &addressStr, BluetoothDevice::Pairing pairing);
    BluetoothDevice::Pairing pairingStatus(const QString &deviceName);
    void killOperation();

private:
    QFutureWatcher<ULONGLONG> watcher;
    ULONGLONG connectTo(const QString &addressStr);
    ULONGLONG disconnectFrom(const QString &addressStr);
    QHash<QString, BluetoothDevice>* devicesList;
    void connectionRequest(const SOCKET &sock, const SOCKADDR_BTH &addr);
    int conReq = -1;
    QFuture<void>* futureReq = nullptr;
    SOCKET localSocket;

signals:
    void deviceDiscovered(const QString &);
    void error(Error err);
    void pairingFinished(ULONGLONG address);
};

#endif // BLUETOOTHLOCALDEVICE_H
