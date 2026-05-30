#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_ANDROID_NO_EXIT_CALL", "1");   // ← add this line

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MobileApp", "Main");
    return QGuiApplication::exec();
}