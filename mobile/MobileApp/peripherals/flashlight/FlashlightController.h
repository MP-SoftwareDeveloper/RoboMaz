#pragma once
#include <QObject>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#endif

class FlashlightController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool flashOn READ flashOn NOTIFY flashOnChanged)

public:
    explicit FlashlightController(QObject *parent = nullptr)
        : QObject(parent), m_blinkTimer(new QTimer(this))
    {
        connect(m_blinkTimer, &QTimer::timeout, this, &FlashlightController::onBlinkTick);
    }

    bool flashOn() const { return m_flashOn; }

public slots:

    // Button 1 — simple on/off toggle
    void toggle() {
        if (m_isBlinking) return; // ignore during blink
        setTorch(!m_flashOn);
    }

    // Button 2 — blink 3 times with 200ms interval
    void blinkThreeTimes() {
        if (m_isBlinking) return; // prevent double trigger
        m_isBlinking = true;
        m_blinkCount = 0;
        m_blinkTimer->start(200);
    }

signals:
    void flashOnChanged();

private slots:
    void onBlinkTick() {
        if (m_blinkCount >= 6) {
            // 6 ticks = 3 on + 3 off
            m_blinkTimer->stop();
            m_isBlinking = false;
            setTorch(false);
            return;
        }
        // odd tick = on, even tick = off
        setTorch(m_blinkCount % 2 == 0);
        m_blinkCount++;
    }

private:
    bool m_flashOn = false;
    bool m_isBlinking = false;
    int m_blinkCount = 0;
    QTimer* m_blinkTimer = nullptr;

    void setTorch(bool on) {
        if (m_flashOn == on) return;
        m_flashOn = on;

#ifdef Q_OS_ANDROID
        QJniObject context = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "getContext",
            "()Landroid/content/Context;"
        );
        if (context.isValid()) {
            QJniObject cameraServiceString = QJniObject::fromString("camera");
            QJniObject cameraManager = context.callObjectMethod(
                "getSystemService",
                "(Ljava/lang/String;)Ljava/lang/Object;",
                cameraServiceString.object<jstring>()
            );
            if (cameraManager.isValid()) {
                QJniObject cameraIdList = cameraManager.callObjectMethod(
                    "getCameraIdList",
                    "()[Ljava/lang/String;"
                );
                QJniEnvironment env;
                jobjectArray array = cameraIdList.object<jobjectArray>();
                if (array && env->GetArrayLength(array) > 0) {
                    jobject cameraId = env->GetObjectArrayElement(array, 0);
                    QJniObject cameraIdObj(cameraId);
                    cameraManager.callMethod<void>(
                        "setTorchMode",
                        "(Ljava/lang/String;Z)V",
                        cameraIdObj.object<jstring>(),
                        (jboolean)on
                    );
                }
            }
        }
#endif
        emit flashOnChanged();
    }
};