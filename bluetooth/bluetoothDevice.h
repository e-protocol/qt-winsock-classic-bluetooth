#pragma once
#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include <winsock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <QString>

class BluetoothDevice
{
public:
    BluetoothDevice(){};

    enum Pairing
    {
        Unpaired						= 0,	// The Bluetooth devices are not paired.
        Paired							= 1,	// The Bluetooth devices are paired.The system will prompt the user for authorization when the remote device initiates a connection to the local device.
        AuthorizedPaired				= 2,	// The Bluetooth devices are paired.The system will not prompt the user for authorization when the remote device initiates a connection to the local device.
    };

    BluetoothDevice(ULONGLONG adrNum, const QString &deviceName, Pairing status);
    const QString &getAddress() { return address; }
    const QString &getName() { return name; }
    ULONGLONG getAddressNum() { return addressNum; }
    Pairing getStatus() { return m_status; }
    void setStatus(Pairing status) { m_status = status; }

private:
    QString address;
    QString name;
    ULONGLONG addressNum;
    Pairing m_status;
};

#endif // BLUETOOTHDEVICE_H
