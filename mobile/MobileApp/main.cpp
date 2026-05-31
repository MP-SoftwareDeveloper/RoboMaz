#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QQmlContext>
#include "flashlightcontroller.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_ANDROID_NO_EXIT_CALL", "1");

    QGuiApplication app(argc, argv);

    FlashlightController flashlight;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("flashlight", &flashlight);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("MobileApp", "Main");
    return QGuiApplication::exec();
}