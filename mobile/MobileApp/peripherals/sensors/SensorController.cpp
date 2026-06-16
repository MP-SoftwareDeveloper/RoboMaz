#include "SensorController.h"
#include <QtMath>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

SensorController::SensorController(QObject *parent)
    : QObject(parent)
    , m_accel(new QAccelerometer(this))
    , m_compass(new QCompass(this))
{
    m_accel->setDataRate(20);
    m_compass->setDataRate(10);

    connect(m_accel,   &QAccelerometer::readingChanged,
            this,      &SensorController::onAccelReading);
    connect(m_compass, &QCompass::readingChanged,
            this,      &SensorController::onCompassReading);

    // Defer first attempt: Android sensor backend needs the event loop running
    // and the Activity fully initialized before JNI sensor calls work.
    // tryStart() retries every 500 ms for up to 5 s if the first attempt fails.
    QTimer::singleShot(200, this, &SensorController::tryStart);
}

// ── Sensor startup with retry ─────────────────────────────────────────────────
// Sensor plugins are pre-loaded by the Android Java layer via load_local_libs in
// libs.xml. All we need to do here is call start() once the event loop is running.

void SensorController::tryStart()
{
    m_startAttempts++;

    // Collect what Qt's sensor plugin system has registered
    QList<QByteArray> types    = QSensor::sensorTypes();
    QList<QByteArray> backends = QSensor::sensorsForType(QAccelerometer::sensorType);

    // Use qInfo() — qDebug() is filtered on MIUI; Info always reaches logcat
    qInfo() << "[Sensor] attempt" << m_startAttempts
            << "types:" << types
            << "accelBackends:" << backends;

    bool accelOk   = m_accel->start();
    bool compassOk = m_compass->start();

    qInfo() << "[Sensor] start() -> accel:" << accelOk << " compass:" << compassOk;

    // Update the on-screen status string so we can diagnose without logcat
    m_statusText = QString("attempt %1 | types: %2 | accel backends: %3 | start: %4/%5")
                   .arg(m_startAttempts)
                   .arg(types.size())
                   .arg(backends.size())
                   .arg(accelOk ? "OK" : "FAIL")
                   .arg(compassOk ? "OK" : "FAIL");

    if (accelOk) {
        m_sensorsActive = true;
        emit sensorsActiveChanged();
        return;
    }

    emit sensorsActiveChanged();   // update statusText in QML even on failure

    if (m_startAttempts < 10) {
        QTimer::singleShot(500, this, &SensorController::tryStart);
    } else {
        qInfo() << "[Sensor] FAILED after 10 attempts";
        m_statusText = QString("FAILED | types:%1 backends:%2")
                       .arg(types.size()).arg(backends.size());
        emit sensorsActiveChanged();
    }
}

// ── Sensor callbacks ─────────────────────────────────────────────────────────

void SensorController::onAccelReading()
{
    QAccelerometerReading *r = m_accel->reading();
    if (!r) return;

    m_ax = r->x();
    m_ay = r->y();
    m_az = r->z();

    // Pitch: tilt forward/backward
    m_pitch = qRadiansToDegrees(qAtan2(m_ay, qSqrt(m_ax*m_ax + m_az*m_az)));
    // Roll: tilt left/right  (no qAbs — must preserve sign of az for face-down)
    m_roll  = qRadiansToDegrees(qAtan2(-m_ax, m_az));

    emit pitchChanged();
    emit rollChanged();
}

void SensorController::onCompassReading()
{
    // QCompass uses Android's sensor fusion internally — tilt-compensated,
    // calibrated heading. No manual cross-product needed.
    QCompassReading *r = m_compass->reading();
    if (!r) return;

    double az = r->azimuth();
    if (az < 0.0) az += 360.0;

    if (qAbs(az - m_azimuth) > 0.2) {
        m_azimuth = az;
        emit azimuthChanged();
    }
}

// ── String helpers ────────────────────────────────────────────────────────────

QString SensorController::pitchStr() const
{
    return QString::number(m_pitch, 'f', 1) + QStringLiteral("°");
}

QString SensorController::rollStr() const
{
    return QString::number(m_roll, 'f', 1) + QStringLiteral("°");
}

QString SensorController::azimuthStr() const
{
    return QString::number(m_azimuth, 'f', 1) + QStringLiteral("°");
}

QString SensorController::cardinal() const
{
    double a = m_azimuth;
    if (a < 22.5  || a >= 337.5) return QStringLiteral("N");
    if (a < 67.5)                 return QStringLiteral("NE");
    if (a < 112.5)                return QStringLiteral("E");
    if (a < 157.5)                return QStringLiteral("SE");
    if (a < 202.5)                return QStringLiteral("S");
    if (a < 247.5)                return QStringLiteral("SW");
    if (a < 292.5)                return QStringLiteral("W");
    return                               QStringLiteral("NW");
}
