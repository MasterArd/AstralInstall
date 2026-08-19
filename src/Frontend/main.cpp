#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include "gamemanager.h"
#include "backendbridge.h"
#include "settings.h"
#include "testconsole.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Der native Windows-Style zeichnet seine eigenen Indikatoren und
    // ignoriert Anpassungen (z.B. in FilterCheckBox.qml). "Basic" ist
    // voll anpassbar und sieht auf Windows und Linux gleich aus.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCoreApplication::setOrganizationName(QStringLiteral("AstralInstall"));
    QCoreApplication::setApplicationName(QStringLiteral("AstralInstall"));

    GameManager gameManager;
    BackendBridge backend;
    Settings settings;
    TestConsole testConsole;


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
    engine.rootContext()->setContextProperty("testConsole", &testConsole);

    // Registered as a proper QML singleton rather than a context
    // property, so Colors.qml can import it.
    qmlRegisterSingletonInstance("AstralInstall", 1, 0, "Settings", &settings);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
