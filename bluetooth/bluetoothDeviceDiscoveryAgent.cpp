#include "bluetoothDeviceDiscoveryAgent.h"

BluetoothDeviceDiscoveryAgent::BluetoothDeviceDiscoveryAgent(BluetoothLocalDevice* localDevice)
{
    m_localDevice = localDevice;
    future = new QFuture<bool>;

    QObject::connect(&watcher, &QFutureWatcher<bool>::finished, [this]()
    {
        bool thisWorked = watcher.future().result();
        if (!thisWorked)
            emit error(UnknownError);
        else
            emit finished();

        activeState = false;
    });
}

void BluetoothDeviceDiscoveryAgent::start()
{
    if(future != nullptr)
        delete future;

    future = new QFuture<bool>(QtConcurrent::run(m_localDevice, &BluetoothLocalDevice::enumirateDevices));
    watcher.setFuture(*future);
    activeState = true;
}

void BluetoothDeviceDiscoveryAgent::stop()
{
    m_localDevice->isCycle = false;
}

void BluetoothDeviceDiscoveryAgent::stopThread()
{
    m_localDevice->isCycle = false;

    if(future != nullptr)
        delete future;
}
