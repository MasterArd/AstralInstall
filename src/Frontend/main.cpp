#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}