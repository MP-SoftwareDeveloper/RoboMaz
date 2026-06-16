#pragma once

#include <QObject>
#include <QStringList>

#ifdef HAS_QT_BLUETOOTH
#  include <QBluetoothDeviceDiscoveryAgent>
#  include <QBluetoothDeviceInfo>
#  include <QBluetoothLocalDevice>
#endif

class BluetoothController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool        scanning    MEMBER m_scanning    NOTIFY scanningChanged)
    Q_PROPERTY(bool        bluetoothOn READ   bluetoothOn   NOTIFY bluetoothOnChanged)
    Q_PROPERTY(QStringList devices     MEMBER m_devices     NOTIFY devicesChanged)
    Q_PROPERTY(QString     status      MEMBER m_status      NOTIFY statusChanged)

public:
    explicit BluetoothController(QObject *parent = nullptr);

    bool bluetoothOn() const;

public slots:
    void startScan();
    void stopScan();

signals:
    void scanningChanged();
    void bluetoothOnChanged();
    void devicesChanged();
    void statusChanged();

private slots:
#ifdef Q_OS_ANDROID
    void onPermissionResult();
#endif
#ifdef HAS_QT_BLUETOOTH
    void doStartScan();
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);
#endif

private:
    bool        m_scanning = false;
    QStringList m_devices;
    QString     m_status;

#ifdef HAS_QT_BLUETOOTH
    QBluetoothDeviceDiscoveryAgent *m_agent = nullptr;
    QBluetoothLocalDevice          *m_local = nullptr;
#endif
};
