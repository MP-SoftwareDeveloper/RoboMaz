#pragma once

#include <QObject>
#include <QAccelerometer>
#include <QCompass>

class SensorController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double  pitch        READ pitch        NOTIFY pitchChanged)
    Q_PROPERTY(double  roll         READ roll         NOTIFY rollChanged)
    Q_PROPERTY(double  azimuth      READ azimuth      NOTIFY azimuthChanged)
    Q_PROPERTY(QString pitchStr     READ pitchStr     NOTIFY pitchChanged)
    Q_PROPERTY(QString rollStr      READ rollStr      NOTIFY rollChanged)
    Q_PROPERTY(QString azimuthStr   READ azimuthStr   NOTIFY azimuthChanged)
    Q_PROPERTY(QString cardinal     READ cardinal     NOTIFY azimuthChanged)
    Q_PROPERTY(bool    sensorsActive READ sensorsActive NOTIFY sensorsActiveChanged)
    Q_PROPERTY(QString statusText   READ statusText   NOTIFY sensorsActiveChanged)

public:
    explicit SensorController(QObject *parent = nullptr);

    double  pitch()        const { return m_pitch; }
    double  roll()         const { return m_roll;  }
    double  azimuth()      const { return m_azimuth; }
    bool    sensorsActive() const { return m_sensorsActive; }
    QString statusText()   const { return m_statusText;   }
    QString pitchStr()     const;
    QString rollStr()      const;
    QString azimuthStr()   const;
    QString cardinal()     const;

signals:
    void pitchChanged();
    void rollChanged();
    void azimuthChanged();
    void sensorsActiveChanged();

private slots:
    void onAccelReading();
    void onCompassReading();

private:
    void tryStart();

    QAccelerometer *m_accel   = nullptr;
    QCompass       *m_compass = nullptr;

    // Accelerometer raw readings (used for pitch/roll)
    double m_ax = 0.0, m_ay = 0.0, m_az = 9.8;

    double m_pitch         = 0.0;
    double m_roll          = 0.0;
    double m_azimuth       = 0.0;
    bool    m_sensorsActive = false;
    int     m_startAttempts = 0;
    QString m_statusText    = QStringLiteral("Initialising…");
};
