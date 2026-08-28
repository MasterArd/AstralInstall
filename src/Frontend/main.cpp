#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include "gamemanager.h"
#include "gamelibrary.h"
#include "backendbridge.h"
#include "settings.h"
#include "testconsole.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // The native Windows style draws its own indicators and ignores
    // customizations (e.g. in FilterCheckBox.qml). "Basic" is fully
    // customizable and looks the same on Windows and Linux.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCoreApplication::setOrganizationName(QStringLiteral("AstralInstall"));
    QCoreApplication::setApplicationName(QStringLiteral("AstralInstall"));

    GameManager gameManager;
    GameLibrary gameLibrary;
    BackendBridge backend;
    Settings settings;
    // The console sends its commands over the same bridge as the UI.
    TestConsole testConsole(&backend);


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

    // TEMPORARY: the library is mocked from a JSON file next to the
    // executable. Replace this with the backend's "list_games" answer
    // once it exists - gameLibrary.setGames() takes it as is.
    gameLibrary.loadFromFile(GameLibrary::defaultFilePath());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameManager", &gameManager);
    engine.rootContext()->setContextProperty("gameLibrary", &gameLibrary);
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
