#pragma once

#include <QObject>
#include <QAccelerometer>
#include <QCompass>

class SensorController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double  pitch      READ pitch      NOTIFY pitchChanged)
    Q_PROPERTY(double  roll       READ roll       NOTIFY rollChanged)
    Q_PROPERTY(double  azimuth    READ azimuth    NOTIFY azimuthChanged)
    Q_PROPERTY(QString pitchStr   READ pitchStr   NOTIFY pitchChanged)
    Q_PROPERTY(QString rollStr    READ rollStr    NOTIFY rollChanged)
    Q_PROPERTY(QString azimuthStr READ azimuthStr NOTIFY azimuthChanged)
    Q_PROPERTY(QString cardinal   READ cardinal   NOTIFY azimuthChanged)

public:
    explicit SensorController(QObject *parent = nullptr);

    double  pitch()      const { return m_pitch; }
    double  roll()       const { return m_roll;  }
    double  azimuth()    const { return m_azimuth; }
    QString pitchStr()   const;
    QString rollStr()    const;
    QString azimuthStr() const;
    QString cardinal()   const;

signals:
    void pitchChanged();
    void rollChanged();
    void azimuthChanged();

private slots:
    void onAccelReading();
    void onCompassReading();

private:
    QAccelerometer *m_accel   = nullptr;
    QCompass       *m_compass = nullptr;

    double m_pitch   = 0.0;
    double m_roll    = 0.0;
    double m_azimuth = 0.0;
};
