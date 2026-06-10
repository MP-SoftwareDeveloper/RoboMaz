#include "SensorController.h"
#include <QtMath>

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

    m_accel->start();
    m_compass->start();
}

void SensorController::onAccelReading()
{
    QAccelerometerReading *r = m_accel->reading();
    if (!r) return;

    double ax = r->x();
    double ay = r->y();
    double az = r->z();

    double newPitch = qRadiansToDegrees(qAtan2(ay, qSqrt(ax*ax + az*az)));
    double newRoll  = qRadiansToDegrees(qAtan2(-ax, qAbs(az)));

    if (!qFuzzyCompare(m_pitch, newPitch)) { m_pitch = newPitch; emit pitchChanged(); }
    if (!qFuzzyCompare(m_roll,  newRoll))  { m_roll  = newRoll;  emit rollChanged();  }
}

void SensorController::onCompassReading()
{
    QCompassReading *r = m_compass->reading();
    if (!r) return;

    double newAzimuth = r->azimuth();
    if (!qFuzzyCompare(m_azimuth, newAzimuth)) {
        m_azimuth = newAzimuth;
        emit azimuthChanged();
    }
}

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
    return QStringLiteral("NW");
}
