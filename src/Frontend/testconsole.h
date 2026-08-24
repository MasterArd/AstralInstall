#ifndef TESTCONSOLE_H
#define TESTCONSOLE_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QStringList>

class BackendBridge;

class TestConsole : public QDialog {
    Q_OBJECT

public:
    // Der Bridge-Zeiger darf null sein - dann laufen nur die lokalen
    // Befehle, die Backend-Befehle melden sich mit einer Fehlerzeile.
    explicit TestConsole(BackendBridge *bridge = nullptr, QWidget *parent = nullptr);

private slots:
    void onReturnPressed();

private:
    void processCommand(const QString &cmd);
    void handleCheck(const QStringList &args);
    void handleDownload(QStringList args);
    void print(const QString &text);

    // Zerlegt eine Eingabezeile in Tokens. Anfuehrungszeichen halten
    // zusammen, was zusammengehoert: dest="C:\Program Files\Games".
    static QStringList tokenize(const QString &line);

    // Sucht "name=wert" in den Argumenten und entfernt den Treffer aus
    // der Liste. Gibt fallback zurueck, wenn nichts passt.
    static QString takeOption(QStringList &args, const QString &name,
                              const QString &fallback = QString());

    // "windows", "linux" oder "darwin" - passend zum laufenden System.
    static QString currentPlatform();

    // Downloads/AstralInstall, falls der Nutzer kein dest= angibt.
    static QString defaultDestination();

    QPlainTextEdit *output;
    QLineEdit *input;
    BackendBridge *backend;
};

#endif // TESTCONSOLE_H
