#include "bluetoothDevice.h"
#include <QString>
#include <QDebug>

BluetoothDevice::BluetoothDevice(ULONGLONG adrNum, const QString &deviceName, Pairing status)
{
    addressNum = adrNum;
    name = deviceName;
    address = QString::number(adrNum);
    m_status = status;
}
