#pragma once
#ifndef BLUETOOTHDEVICEDISCOVERYAGENT_H
#define BLUETOOTHDEVICEDISCOVERYAGENT_H

#include "bluetoothLocalDevice.h"
#include <QList>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

class BluetoothDeviceDiscoveryAgent : public QObject
{
    Q_OBJECT
public:
    BluetoothDeviceDiscoveryAgent(BluetoothLocalDevice* localDevice);

    enum DiscoveryMethod
    {
        NoMethod			= 0x00,		// The discovery is not possible.None of the available methods are supported.
        ClassicMethod		= 0x01,		// The discovery process searches for Bluetooth Classic(BaseRate) devices.
        LowEnergyMethod		= 0x02,		// The discovery process searches for Bluetooth Low Energy devices.
    };
    Q_ENUM(DiscoveryMethod);
    Q_DECLARE_FLAGS(DiscoveryMethods, DiscoveryMethod);

    enum Error
    {
        NoError							= 0,	// No error has occurred.
        InputOutputError				= 1,	// Writing or reading from the device resulted in an error.
        PoweredOffError					= 2,	// The Bluetooth adapter is powered off, power it on before doing discovery.
        InvalidBluetoothAdapterError	= 3,	// The passed local adapter address does not match the physical adapter address of any local Bluetooth device.
        UnsupportedPlatformError		= 4,	// Device discovery is not possible or implemented on the current platform.The error is set in response to a call to start().An example for such cases are iOS versions below 5.0 which do not support Bluetooth device search at all.This value was introduced by Qt 5.5.
        UnsupportedDiscoveryMethod		= 5,	// One of the requested discovery methods is not supported by the current platform.This value was introduced by Qt 5.8.
        UnknownError					= 100,	// An unknown error has occurred.
    };
    Q_ENUM(Error);

    enum InquiryType
    {
        GeneralUnlimitedInquiry			= 0,	// A general unlimited inquiry.Discovers all visible Bluetooth devices in the local vicinity.
        LimitedInquiry					= 1,	// A limited inquiry discovers devices that are in limited inquiry mode.
    };
    Q_ENUM(InquiryType);

    void start();
    void stop();
    bool isActive() const { return activeState; }
    void stopThread();

private:
    BluetoothLocalDevice* m_localDevice;
    QFutureWatcher<bool> watcher;
    bool activeState = false;
    QFuture<bool>* future;

signals:
    void deviceDiscovered(const BluetoothDevice &device);
    void error(BluetoothDeviceDiscoveryAgent::Error error);
    void finished();
};

#endif // BLUETOOTHDEVICEDISCOVERYAGENT_H
