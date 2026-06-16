#include "BluetoothController.h"
#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#  include <QPermissions>
#endif

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
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this,    &BluetoothController::onScanError);
    connect(m_local, &QBluetoothLocalDevice::hostModeStateChanged,
            this, [this](QBluetoothLocalDevice::HostMode) {
        emit bluetoothOnChanged();
        m_status = bluetoothOn() ? "Ready" : "Bluetooth off";
        emit statusChanged();
    });
#else
    m_status = "Qt Bluetooth not installed.\n"
               "Add via Qt Maintenance Tool → Qt 6.x → Android arm64-v8a → Qt Bluetooth.";
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

// ── Public slots ─────────────────────────────────────────────────────────────

void BluetoothController::startScan()
{
#ifdef HAS_QT_BLUETOOTH

#ifdef Q_OS_ANDROID
    // Android 6+: dangerous permissions must be requested at runtime.
    // QBluetoothPermission covers BLUETOOTH_SCAN + BLUETOOTH_CONNECT (API 31+)
    // and ACCESS_FINE_LOCATION (API < 31) automatically.
    QBluetoothPermission perm;
    switch (qApp->checkPermission(perm)) {
    case Qt::PermissionStatus::Undetermined:
        m_status = "Requesting Bluetooth permission…";
        emit statusChanged();
        qApp->requestPermission(perm, this, &BluetoothController::onPermissionResult);
        return;
    case Qt::PermissionStatus::Denied:
        m_status = "Bluetooth permission denied — enable it in Settings";
        emit statusChanged();
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif // Q_OS_ANDROID

    doStartScan();

#endif // HAS_QT_BLUETOOTH
}

void BluetoothController::stopScan()
{
#ifdef HAS_QT_BLUETOOTH
    if (m_agent) m_agent->stop();
#endif
}

// ── Private slots ─────────────────────────────────────────────────────────────

#ifdef Q_OS_ANDROID
void BluetoothController::onPermissionResult()
{
#ifdef HAS_QT_BLUETOOTH
    QBluetoothPermission perm;
    if (qApp->checkPermission(perm) == Qt::PermissionStatus::Granted) {
        doStartScan();
    } else {
        m_status = "Bluetooth permission denied — enable it in Settings";
        emit statusChanged();
    }
#endif
}
#endif // Q_OS_ANDROID

#ifdef HAS_QT_BLUETOOTH

void BluetoothController::doStartScan()
{
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
    // BLE-only — robot is a BLE peripheral; Classic scan needs extra permissions
    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

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
    m_status = "Scan error: " + m_agent->errorString();
    emit statusChanged();
}

#endif // HAS_QT_BLUETOOTH
