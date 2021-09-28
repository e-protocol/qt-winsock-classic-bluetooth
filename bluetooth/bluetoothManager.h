#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QListWidget>
#include "../dialogs/messageBox.h"
#include <QPushButton>
#include "../bluetooth/bluetoothLocalDevice.h"
#include "../bluetooth/bluetoothDeviceDiscoveryAgent.h"
#include "../bluetooth/bluetoothDevice.h"
#include "../bluetooth/bluetoothCall.h"

class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    BluetoothManager();
    QListWidget* getListWidget() { return devicesListWidget; }
    void setListWidget(QListWidget* listWidget);
    bool checkLocalDevice();
    void agentStart();
    void agentStop();
    void shutdownSocket(bool allSockets);

private:
    QListWidget* devicesListWidget;
    QList<QListWidgetItem *> *list_items;
    BluetoothLocalDevice* localDevice = nullptr;
    BluetoothDeviceDiscoveryAgent* agent = nullptr;
    void setItemColor(QListWidgetItem &item, BluetoothDevice::Pairing pairingStatus);
    void errorMessage(const QString &message);
    void dialogPairing(const QString &addressStr);
    void dialogDisconnect(const QString &addressStr);
    //void dialogConnect(const QString &address);

private slots:
    void addDevice(const QString &deviceName);
    void listWidgetClicked(QListWidgetItem *item);
    void scanFinished();
    void pairingDone(ULONGLONG address);
    void enableList(BluetoothLocalDevice::Error error);
    //void socketConnected(const BluetoothAddress& address);
    void readData();
    //void connectionLost(BluetoothSocket::SocketError error);

signals:
    void localDeviceStatusChanged();
    void setScanStatus(bool status);
    void sendMessage(const QString &message);
    void pairingComplete();
    void pairingStart(const QString &text);
};

#endif // BLUETOOTHMANAGER_H
