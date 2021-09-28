#include "bluetoothLocalDevice.h"
#include "bluetoothCall.h"
#include <QDebug>
#include <winsock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <QString>

BluetoothLocalDevice::BluetoothLocalDevice()
{
    devicesList = new QHash<QString, BluetoothDevice>;
    qRegisterMetaType<ULONGLONG>("ULONGLONG");
    QObject::connect(&watcher, &QFutureWatcher<ULONGLONG>::finished, [this]()
    {
        ULONGLONG thisWorked = watcher.future().result();
        conReq = -1;

        if(futureReq != nullptr)
        {
            delete futureReq;
            futureReq = nullptr;
        }

        BluetoothCall::callShutdown(localSocket);
        closesocket(localSocket);
        WSACleanup();

        if(isKill)
            thisWorked = 0;

        switch(thisWorked)
        {
            case(0) : emit error(NoError); break;
            case(1) : emit error(WinsockError); break;
            case(2) : emit error(RemoteAddressError); break;
            case(3) : emit error(SocketError); break;
            case(4) : emit error(PairingError); break;
            default:
            {
                for(auto &elem : *devicesList)
                    if(elem.getAddressNum() == thisWorked)
                    {
                        emit pairingFinished(thisWorked);
                        break;
                    }

                break;
            }
        }
    });
}

bool BluetoothLocalDevice::isValid() const
{
    BLUETOOTH_FIND_RADIO_PARAMS btfrp;
    btfrp.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);
    HANDLE radio;

    // Get the first local radio
    HBLUETOOTH_RADIO_FIND radioFindHandle = BluetoothFindFirstRadio(&btfrp, &radio);
    if (radioFindHandle)
        return true;

    return false;
}

bool BluetoothLocalDevice::enumirateDevices()
{
    ULONG ulRetCode = 0;
    WSADATA wSAData = {0};
    ulRetCode = WSAStartup(MAKEWORD(2, 2), &wSAData);

    //Check for Winsock version 2.2.
    if (ulRetCode != 0)
        return false;

    isCycle = true;
    devicesList->clear();
    WSAQUERYSET querySet;
    memset(&querySet, 0, sizeof(querySet));
    querySet.dwSize = sizeof(querySet);
    querySet.dwNameSpace = NS_BTH;
    HANDLE hLookup;

    if(0 != WSALookupServiceBegin(&querySet, LUP_CONTAINERS | LUP_FLUSHCACHE, &hLookup))
    {
        if(WSAGetLastError() != WSASERVICE_NOT_FOUND)
        {
            // error during WSALookupServiceBegin
        }
        else
        {
            //No BlueTooth device Found
        }
        return false;
    }

    DWORD deviceLength = 2000;
    char buf[deviceLength];
    WSAQUERYSET* pDevice = PWSAQUERYSET(buf);
    DWORD flags = LUP_RETURN_ADDR | LUP_RETURN_NAME | LUP_RETURN_BLOB | LUP_FLUSHCACHE;

    while (0 == WSALookupServiceNext(hLookup, flags, &deviceLength, pDevice))
    {
        if(!isCycle)
        {
            WSALookupServiceEnd(hLookup);
            WSACleanup();
            return true;
        }

        PSOCKADDR_BTH sa = PSOCKADDR_BTH(pDevice->lpcsaBuffer->RemoteAddr.lpSockaddr);

        if(sa->addressFamily != AF_BTH)
        {
           // Address family is not AF_BTH  for bluetooth device discovered
            continue;
        }

        //the name is available in pDevice->lpszServiceInstanceName
        //the MAC address is available in sa->btAddr
        QString deviceName = QString::fromWCharArray(pDevice->lpszServiceInstanceName);

        if(deviceName.isEmpty())
            deviceName = "(без имени)";

        unsigned long statusConnected = (pDevice->dwOutputFlags & BTHNS_RESULT_DEVICE_CONNECTED ) > 0;
        unsigned long statusAuthenticated = (pDevice->dwOutputFlags & BTHNS_RESULT_DEVICE_AUTHENTICATED) > 0;
        BluetoothDevice::Pairing pairingStatus;

        if(statusAuthenticated + statusConnected > 1)
            pairingStatus = BluetoothDevice::Paired;
        else
            pairingStatus = BluetoothDevice::Unpaired;

        BluetoothDevice d(sa->btAddr, deviceName, pairingStatus);
        devicesList->insert(deviceName + "/" + QString::number(sa->btAddr), d);
        emit deviceDiscovered(deviceName + "/" + QString::number(sa->btAddr));
    }

    WSALookupServiceEnd(hLookup);
    WSACleanup();
    return true;
}

void BluetoothLocalDevice::requestPairing(const QString &addressStr, BluetoothDevice::Pairing pairing)
{
    if(pairing == BluetoothDevice::Paired)
    {
        QFuture<ULONGLONG> future = QtConcurrent::run(this, &BluetoothLocalDevice::connectTo, addressStr);
        watcher.setFuture(future);
    }
    else if(pairing == BluetoothDevice::Unpaired)
    {
        QFuture<ULONGLONG> future = QtConcurrent::run(this, &BluetoothLocalDevice::disconnectFrom, addressStr);
        watcher.setFuture(future);
    }
}

ULONGLONG BluetoothLocalDevice::connectTo(const QString &addressStr)
{
    ULONG ulRetCode = 0;
    WSADATA wSAData = {0};
    ulRetCode = WSAStartup(MAKEWORD(2, 2), &wSAData);

    //Check for Winsock version 2.2.
    if (ulRetCode != 0)
        return 1;

    SOCKADDR_BTH sockAddrBthServer = {0};
    ULONGLONG ululRemoteAddr = 0;

    for(auto &elem : *devicesList)
        if(elem.getAddress() == addressStr)
        {
            ululRemoteAddr = elem.getAddressNum();
            break;
        }

    if(ululRemoteAddr == 0)
        return 2;

    int i = 1;
    localSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    setsockopt(localSocket,SOL_RFCOMM,SO_BTH_AUTHENTICATE,(char*)&i,(int)sizeof(i));
    sockAddrBthServer.addressFamily = AF_BTH;
    sockAddrBthServer.serviceClassId = SerialPortServiceClass_UUID; //RFCOMM_PROTOCOL_UUID;
    sockAddrBthServer.btAddr = (BTH_ADDR) ululRemoteAddr;
    sockAddrBthServer.port = BT_PORT_ANY;

    if(localSocket == INVALID_SOCKET)
        return 3;

    futureReq = new QFuture<void>(QtConcurrent::run(this, &BluetoothLocalDevice::connectionRequest,localSocket,sockAddrBthServer));

    while(true)
    {
        if(isKill)
            return 0;

        if(conReq > 0)
            return conReq;
        else if(conReq == 0)
            break;
    }

    for(auto &elem : *devicesList)
        if(elem.getAddressNum() == ululRemoteAddr)
        {
            elem.setStatus(BluetoothDevice::Paired);
            break;
        }

    return ululRemoteAddr;
}

BluetoothDevice::Pairing BluetoothLocalDevice::pairingStatus(const QString &deviceName)
{
    auto it = devicesList->find(deviceName);

    if(it != devicesList->end())
        return it.value().getStatus();

    return BluetoothDevice::Unpaired;
}

ULONGLONG BluetoothLocalDevice::disconnectFrom(const QString &addressStr)
{
    ULONGLONG ululRemoteAddr = 0;

    for(auto &elem : *devicesList)
        if(elem.getAddress() == addressStr)
        {
            ululRemoteAddr = elem.getAddressNum();
            break;
        }

    if(ululRemoteAddr == 0)
        return 2;

    BLUETOOTH_ADDRESS addr;
    addr.ullLong = ululRemoteAddr;

    for(int k = 0 ; k < 6; k++)
         addr.rgBytes[k] = ((ululRemoteAddr >> (8 * k)) & 0XFF);

    auto ret = BluetoothRemoveDevice(&addr);

    if(ret != ERROR_SUCCESS)
        return 4;

    for(auto &elem : *devicesList)
        if(elem.getAddressNum() == ululRemoteAddr)
        {
            elem.setStatus(BluetoothDevice::Unpaired);
            break;
        }

    return ululRemoteAddr;
}

void BluetoothLocalDevice::killOperation()
{
    //futureReq->cancel();
    watcher.waitForFinished();
    //watcher.cancel();
}

void BluetoothLocalDevice::connectionRequest(const SOCKET &sock, const SOCKADDR_BTH &addr)
{
    WSASetLastError(0);

    if(BluetoothCall::callConnect(sock, addr) == SOCKET_ERROR || WSAGetLastError() != 0)
        conReq = 4;
    else
        conReq = 0;
}
