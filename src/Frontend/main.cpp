#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "gamemanager.h"
#include "backendbridge.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    GameManager gameManager;
    BackendBridge backend;

    QObject::connect(&backend, &BackendBridge::releaseChecked,
                     [](const QString &repo, const QString &version) {
                         qInfo("[IPC] %s -> %s",
                               qUtf8Printable(repo), qUtf8Printable(version));
                     });
    QObject::connect(&backend, &BackendBridge::requestFailed,
                     [](const QString &repo, const QString &error) {
                         qWarning("[IPC] %s failed: %s",
                                  qUtf8Printable(repo), qUtf8Printable(error));
                     });
    QObject::connect(&backend, &BackendBridge::backendError,
                     [](const QString &message) {
                         qWarning("[IPC] %s", qUtf8Printable(message));
                     });

    // Fork the backend as a child process before the UI comes up.
    backend.start();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameManager", &gameManager);
    engine.rootContext()->setContextProperty("backend", &backend);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
