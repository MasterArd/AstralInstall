#ifndef TESTCONSOLE_H
#define TESTCONSOLE_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QStringList>

class BackendBridge;
class InstallManager;

class TestConsole : public QDialog {
    Q_OBJECT

public:
    // The bridge pointer may be null - then only the local commands
    // run, and the backend commands report back with an error line.
    explicit TestConsole(BackendBridge *bridge = nullptr,
                         InstallManager *installer = nullptr,
                         QWidget *parent = nullptr);

private slots:
    void onReturnPressed();

private:
    void processCommand(const QString &cmd);
    void handleCheck(const QStringList &args);
    void handleDownload(QStringList args);
    void handleInstall(const QStringList &args);
    void handleInstallZip(QStringList args);
    void handleGameCommand(const QString &command, QStringList args);
    void printGames();
    void print(const QString &text);

    // Splits an input line into tokens. Quotes keep together what
    // belongs together: dest="C:\Program Files\Games".
    static QStringList tokenize(const QString &line);

    // Looks for "name=value" in the arguments and removes the match
    // from the list. Returns fallback if nothing matches.
    static QString takeOption(QStringList &args, const QString &name,
                              const QString &fallback = QString());

    // "windows", "linux" or "darwin" - matching the running system.
    static QString currentPlatform();

    // Downloads/AstralInstall, if the user does not pass a dest=.
    static QString defaultDestination();

    QPlainTextEdit *output;
    QLineEdit *input;
    BackendBridge *backend;
    InstallManager *installer;
};

#endif // TESTCONSOLE_H
