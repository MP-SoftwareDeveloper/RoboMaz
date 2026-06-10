package org.qtproject.example.appMobileApp;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

/**
 * SensorBridge — registered as an Android SensorEventListener.
 * C++ polls getAccelX/Y/Z() and getAzimuth() via QJniObject every ~50 ms.
 */
public class SensorBridge implements SensorEventListener {

    private static SensorBridge s_instance;
    private final SensorManager m_sensorManager;

    // Latest raw sensor readings (protected by synchronized getters)
    private final float[] m_accel  = { 0f, 0f, 9.8f };
    private final float[] m_magnet = { 0f, 0f, 0f   };

    // ── Singleton ─────────────────────────────────────────────────────────────
    public static synchronized SensorBridge getInstance(Context context) {
        if (s_instance == null)
            s_instance = new SensorBridge(context.getApplicationContext());
        return s_instance;
    }

    private SensorBridge(Context context) {
        m_sensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);

        Sensor accel  = m_sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        Sensor magnet = m_sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);

        if (accel  != null)
            m_sensorManager.registerListener(this, accel,  SensorManager.SENSOR_DELAY_UI);
        if (magnet != null)
            m_sensorManager.registerListener(this, magnet, SensorManager.SENSOR_DELAY_UI);
    }

    // ── SensorEventListener ───────────────────────────────────────────────────
    @Override
    public void onSensorChanged(SensorEvent event) {
        synchronized (this) {
            switch (event.sensor.getType()) {
                case Sensor.TYPE_ACCELEROMETER:
                    System.arraycopy(event.values, 0, m_accel,  0, 3);
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD:
                    System.arraycopy(event.values, 0, m_magnet, 0, 3);
                    break;
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) { /* unused */ }

    // ── C++ polling interface ─────────────────────────────────────────────────
    public synchronized float getAccelX() { return m_accel[0]; }
    public synchronized float getAccelY() { return m_accel[1]; }
    public synchronized float getAccelZ() { return m_accel[2]; }

    /**
     * Returns the magnetic north azimuth (0–360°) computed from
     * accelerometer + magnetometer using SensorManager.getOrientation().
     */
    public synchronized float getAzimuth() {
        float[] R = new float[9];
        float[] I = new float[9];
        float[] accelCopy  = m_accel.clone();
        float[] magnetCopy = m_magnet.clone();

        if (SensorManager.getRotationMatrix(R, I, accelCopy, magnetCopy)) {
            float[] orientation = new float[3];
            SensorManager.getOrientation(R, orientation);
            float az = (float) Math.toDegrees(orientation[0]);
            return az < 0f ? az + 360f : az;
        }
        return 0f;
    }
}
