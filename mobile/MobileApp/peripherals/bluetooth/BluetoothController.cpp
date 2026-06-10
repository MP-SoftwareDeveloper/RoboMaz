#include "BluetoothController.h"

BluetoothController::BluetoothController(QObject *parent)
    : QObject(parent)
{
#ifdef HAS_QT_BLUETOOTH
    m_agent = new QBluetoothDeviceDiscoveryAgent(this);
    m_local = new QBluetoothLocalDevice(this);

    m_status = bluetoothOn() ? "Ready" : "Bluetooth off";

    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this,    &BluetoothController::onDeviceDiscovered);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished,
            this,    &BluetoothController::onScanFinished);
    connect(m_agent,
            QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(
                &QBluetoothDeviceDiscoveryAgent::errorOccurred),
            this, &BluetoothController::onScanError);
    connect(m_local, &QBluetoothLocalDevice::hostModeStateChanged,
            this, [this](QBluetoothLocalDevice::HostMode) {
        emit bluetoothOnChanged();
        m_status = bluetoothOn() ? "Ready" : "Bluetooth off";
        emit statusChanged();
    });
#else
    m_status = "Install Qt Bluetooth via Qt Maintenance Tool\n"
               "Qt 6.11.1 → Android arm64-v8a → Qt Bluetooth";
#endif
}

bool BluetoothController::bluetoothOn() const
{
#ifdef HAS_QT_BLUETOOTH
    return m_local && m_local->isValid() &&
           m_local->hostMode() != QBluetoothLocalDevice::HostPoweredOff;
#else
    return false;
#endif
}

void BluetoothController::startScan()
{
#ifdef HAS_QT_BLUETOOTH
    if (!bluetoothOn()) {
        m_status = "Please enable Bluetooth";
        emit statusChanged();
        return;
    }
    m_devices.clear();
    emit devicesChanged();
    m_scanning = true;
    emit scanningChanged();
    m_status = "Scanning…";
    emit statusChanged();
    m_agent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                   QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#endif
}

void BluetoothController::stopScan()
{
#ifdef HAS_QT_BLUETOOTH
    if (m_agent) m_agent->stop();
#endif
}

#ifdef HAS_QT_BLUETOOTH
void BluetoothController::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QString name = device.name().isEmpty()
                       ? device.address().toString()
                       : device.name();
    if (device.rssi() != 0)
        name += "  (" + QString::number(device.rssi()) + " dBm)";
    if (!m_devices.contains(name)) {
        m_devices.append(name);
        emit devicesChanged();
    }
}

void BluetoothController::onScanFinished()
{
    m_scanning = false;
    emit scanningChanged();
    m_status = m_devices.isEmpty() ? "No devices found"
             : QString::number(m_devices.size()) + " device(s) found";
    emit statusChanged();
}

void BluetoothController::onScanError(QBluetoothDeviceDiscoveryAgent::Error)
{
    m_scanning = false;
    emit scanningChanged();
    m_status = "Error: " + m_agent->errorString();
    emit statusChanged();
}
#endif
