#include "bluetoothManager.h"
#include "../bluetooth/bluetoothDevice.h"
#include <QDebug>

BluetoothManager::BluetoothManager()
{
    localDevice = new BluetoothLocalDevice;
    agent = new BluetoothDeviceDiscoveryAgent(localDevice);
    qRegisterMetaType<BluetoothDevice>("");
}

//check if bluetooth module is available
bool BluetoothManager::checkLocalDevice()
{
    return localDevice->isValid();
}

void BluetoothManager::errorMessage(const QString &message)
{
    MsgBox::warning("Ошибка", message);
}

void BluetoothManager::setListWidget(QListWidget *listWidget)
{
    if(localDevice != nullptr)
    {
        delete agent;
        delete localDevice;
        localDevice = new BluetoothLocalDevice;
        agent = new BluetoothDeviceDiscoveryAgent(localDevice);
    }

    //create discovery agent and connect to it's signals
    connect(localDevice, &BluetoothLocalDevice::deviceDiscovered, this,&BluetoothManager::addDevice, Qt::AutoConnection);
    connect(agent, &BluetoothDeviceDiscoveryAgent::finished, this, &BluetoothManager::scanFinished);
    connect(localDevice, &BluetoothLocalDevice::pairingFinished, this, &BluetoothManager::pairingDone);
    connect(localDevice,&BluetoothLocalDevice::error,this,&BluetoothManager::enableList);

    devicesListWidget = listWidget;
    connect(devicesListWidget, &QListWidget::itemClicked, this, &BluetoothManager::listWidgetClicked);
}

void BluetoothManager::readData()
{
    //qDebug() << socket->readAll();
}

void BluetoothManager::agentStart()
{
    //scan for bluetooth devices
    if(!agent->isActive())
        agent->start();

    //update listWidget
    devicesListWidget->clear();
    devicesListWidget->setEnabled(false);
    emit setScanStatus(true);
}

void BluetoothManager::agentStop()
{
    agent->stop();
    //update listWidget
    devicesListWidget->update();
    devicesListWidget->setEnabled(true);
    emit setScanStatus(false);
}

void BluetoothManager::addDevice(const QString &deviceName)
{
    QList<QListWidgetItem *> items = devicesListWidget->findItems(deviceName,Qt::MatchExactly);

    if(items.empty())
    {
        //find devices
        QListWidgetItem *item = new QListWidgetItem(deviceName);
        BluetoothDevice::Pairing pairingStatus = localDevice->pairingStatus(deviceName);
        setItemColor(*item, pairingStatus);

        // check for low energy bluetooth device
        /*if(info.coreConfigurations() & BluetoothDeviceInfo::LowEnergyCoreConfiguration)
        {
            QString type = "  LE";
            item->setText(label + type);
        }*/

        //update listWidget
        devicesListWidget->addItem(item);
    }
}

void BluetoothManager::scanFinished()
{
    if(localDevice->isShutDown)
        return;

    emit setScanStatus(false);
    devicesListWidget->setEnabled(true);
    //show found devices
    if(devicesListWidget->count() == 0)
        MsgBox::critical("Сканирование","Устройства не обнаружены");
}

void BluetoothManager::pairingDone(ULONGLONG address)
{
    //change device color status
    QString name = "";
    BluetoothDevice::Pairing pairingStatus = BluetoothDevice::Unpaired;

    for(auto &elem : *localDevice->getDevicesList())
    {
        if(elem.getAddressNum() == address)
        {
            name = elem.getName() + "/" + elem.getAddress();
            pairingStatus = elem.getStatus();
            break;
        }
    }

    if(name.isEmpty())
        return;

    for(int k = 0; k < devicesListWidget->count(); k++)
        if(devicesListWidget->item(k)->text() == name)
        {
            setItemColor(*devicesListWidget->item(k), pairingStatus);
            break;
        }

    //update listWidget
    devicesListWidget->setEnabled(true); // enable listWidget
    emit pairingComplete();
}

void BluetoothManager::listWidgetClicked(QListWidgetItem *item)
{
    devicesListWidget->setEnabled(false); // disable listWidget

    //pair or unpair device by click
    if(agent->isActive())
    {
        agent->stop(); // stop for scanning
        scanFinished();
    }

    QString name = "";
    QString addressStr = "";

    for(auto &elem : *localDevice->getDevicesList())
    {
        if(elem.getName() + "/" + elem.getAddress() == item->text())
        {
            name = elem.getName() + "/" + elem.getAddress();
            addressStr = elem.getAddress();
            break;
        }
    }

    if(name.isEmpty() || addressStr.isEmpty())
        return;

    BluetoothDevice::Pairing pairingStatus = localDevice->pairingStatus(name);

    switch(pairingStatus)
    {
        /*case BluetoothLocalDevice::AuthorizedPaired:
            dialogDisconnect(address); break;
        case BluetoothLocalDevice::Paired:
            dialogConnect(address); break;*/
        case BluetoothDevice::AuthorizedPaired: case BluetoothDevice::Paired:
            dialogDisconnect(addressStr); break;
        case BluetoothDevice::Unpaired:
            dialogPairing(addressStr); break;
        default:
            break;
    }

    devicesListWidget->clearSelection(); //deselect listWidget
}

/*void BluetoothManager::dialogConnect(const QString &address)
{
    MsgBox msgBox;
    msgBox.setTitle("Cоединение");
    msgBox.setMessage("Создать соединение или \nсбросить сопряжение?");
    msgBox.setButtons(MsgBox::YES, MsgBox::NO, MsgBox::DROP);
    int choice = msgBox.exec();

    switch (choice)
    {
        case MsgBox::YES:
        {
            localDevice->connectTo(address);
            break;
        }
        case MsgBox::NONE: case MsgBox::NO:
        devicesListWidget->setEnabled(true);
        break;
        case MsgBox::DROP:
        {
            localDevice->requestPairing(address,BluetoothLocalDevice::Unpaired);
            break;
        }
        default:
        break;
    }
}*/

void BluetoothManager::dialogPairing(const QString &addressStr)
{
    //Connect to device
    MsgBox* question = new MsgBox;
    question->setTitle("Сопряжение");
    question->setMessage("Создать пару?\n");
    question->setButtons(MsgBox::YES, MsgBox::NO);

    if(question->exec() == MsgBox::YES)
    {
        localDevice->requestPairing(addressStr, BluetoothDevice::Paired);
        emit pairingStart("Сопряжение");
        emit sendMessage("Запрос соединения с устройством по адресу " + addressStr + "...");
    }
    else
        devicesListWidget->setEnabled(true);
}

void BluetoothManager::dialogDisconnect(const QString &addressStr)
{
    //Disconnect to device
    MsgBox* question = new MsgBox;
    question->setTitle("Соединение");
    question->setMessage("Разъединить устройство?\n");
    question->setButtons(MsgBox::YES, MsgBox::NO);

    if(question->exec() == MsgBox::YES)
    {
        QList<QListWidgetItem *> items = devicesListWidget->findItems(addressStr,Qt::MatchExactly);
        emit pairingStart("Разъединение");

        if(!items.empty())
        {
            BluetoothDevice::Pairing pairingStatus = localDevice->pairingStatus(addressStr);

            for(auto &elem : items)
                setItemColor(*elem, pairingStatus);
        }

        //socket->disconnectFromService();
        localDevice->requestPairing(addressStr,BluetoothDevice::Unpaired);
    }
    else
       devicesListWidget->setEnabled(true);
}

/*void BluetoothManager::socketConnected(const BluetoothAddress &address)
{
    QList<QListWidgetItem *> items = devicesListWidget->findItems(address.toString(),Qt::MatchExactly);

    if(!items.empty())
    {
        BluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(address.toString());

        for(auto &elem : items)
            setItemColor(*elem, pairingStatus);
    }

    //check socket connection
    //if(!socket->isOpen())
        //connectionLost(BluetoothSocket::SocketError);
}*/

/*void BluetoothManager::connectionLost(BluetoothSocket::SocketError error)
{
    Q_UNUSED(error);
    MsgBox::critical("Обрыв соединения","Проверьте датчик");
}*/

void BluetoothManager::enableList(BluetoothLocalDevice::Error error)
{
    if(localDevice->isShutDown)
        return;

    //listWidget enable
    devicesListWidget->setEnabled(true);
    emit pairingComplete();

    for(int k = 0; k < devicesListWidget->count(); k++)
    {
        QListWidgetItem *item = devicesListWidget->item(k);
        BluetoothDevice::Pairing pairingStatus = localDevice->pairingStatus(devicesListWidget->item(k)->text());
        setItemColor(*item, pairingStatus);
    }

    QString message;

    switch(error)
    {
        case(0): break;
        case(1): message = "Недоступны службы Windows"; break;
        case(2): message = "Неверный адрес удаленного устройства"; break;
        case(3): message = "Не удалось открыть канал"; break;
        case(4): message = "Не удалось установить сопряжение"; break;
    }

    if(error > 0)
        MsgBox::critical("Ошибка", message);
}

void BluetoothManager::setItemColor(QListWidgetItem &item, BluetoothDevice::Pairing pairingStatus)
{
    switch(pairingStatus)
    {
        case(BluetoothDevice::AuthorizedPaired):
        {
            //item.setForeground(QColor(0x3CB371));
            item.setForeground(QColor(Qt::blue));
            break;
        }
        case(BluetoothDevice::Paired):
        {
            item.setForeground(QColor(Qt::blue));
            break;
        }
        case(BluetoothDevice::Unpaired):
        {
            item.setForeground(QColor("#666666"));
            break;
        }
        default:
            break;
    }
}

//allSockets: true - shutDown, false - kill operations
void BluetoothManager::shutdownSocket(bool allSockets)
{
    if(agent->isActive())
        agent->stopThread();

    if(allSockets)
        localDevice->isShutDown = true;

    localDevice->isKill = true;
    localDevice->killOperation();
}
