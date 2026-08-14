#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "gamemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    GameManager gameManager;
    
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameManager", &gameManager);
    
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}