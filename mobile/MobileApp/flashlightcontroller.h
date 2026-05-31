#pragma once
#include <QObject>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#endif

class FlashlightController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool flashOn READ flashOn NOTIFY flashOnChanged)

public:
    explicit FlashlightController(QObject *parent = nullptr) : QObject(parent) {}

    bool flashOn() const { return m_flashOn; }

public slots:
    void toggle() {
        m_flashOn = !m_flashOn;
#ifdef Q_OS_ANDROID
        // Get CameraManager directly from C++
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
                        (jboolean)m_flashOn
                    );
                }
            }
        }
#endif
        emit flashOnChanged();
    }

signals:
    void flashOnChanged();

private:
    bool m_flashOn = false;
};