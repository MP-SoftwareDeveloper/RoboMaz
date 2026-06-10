#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include "FlashlightController.h"
#include "BluetoothController.h"
#include "SensorController.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_ANDROID_NO_EXIT_CALL", "1");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);

    QGuiApplication app(argc, argv);

    FlashlightController flashlight;
    BluetoothController  bluetooth;
    SensorController     sensors;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("flashlight", &flashlight);
    engine.rootContext()->setContextProperty("bluetooth",  &bluetooth);
    engine.rootContext()->setContextProperty("sensors",    &sensors);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("MobileApp", "Main");
    return QGuiApplication::exec();
}
