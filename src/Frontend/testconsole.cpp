#include "testconsole.h"

#include <QVBoxLayout>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>

#include "backendbridge.h"

TestConsole::TestConsole(BackendBridge *bridge, QWidget *parent)
    : QDialog(parent), backend(bridge)
{
    setWindowTitle("Test Console");
    setGeometry(100, 100, 600, 400);

    // Output area
    output = new QPlainTextEdit();
    output->setReadOnly(true);
    // Nicht "Courier" hart verdrahten - den Namen gibt es auf Linux oft
    // nicht. Qt liefert hier den Monospace-Font des jeweiligen Systems.
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    output->setFont(font);
    output->appendPlainText("=== Test Console ===");
    output->appendPlainText("Type 'help' for commands\n");

    // Input field
    input = new QLineEdit();
    input->setPlaceholderText("Enter command...");
    input->setFont(font);

    connect(input, &QLineEdit::returnPressed, this, &TestConsole::onReturnPressed);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(output, 1);
    layout->addWidget(input);
    setLayout(layout);

    // Focus on input
    input->setFocus();

    // Das Backend antwortet asynchron. Die Antworten landen hier in der
    // Konsole, damit man das Ergebnis eines Befehls auch sieht.
    if (backend) {
        connect(backend, &BackendBridge::releaseChecked, this,
                [this](const QString &repo, const QString &version) {
                    print(QStringLiteral("[ok] %1 -> %2").arg(repo, version));
                });
        connect(backend, &BackendBridge::downloadFinished, this,
                [this](const QString &repo, const QString &destination) {
                    print(QStringLiteral("[ok] %1 heruntergeladen nach %2")
                              .arg(repo, destination));
                });
        connect(backend, &BackendBridge::requestFailed, this,
                [this](const QString &repo, const QString &error) {
                    print(QStringLiteral("[fehler] %1: %2").arg(repo, error));
                });
        connect(backend, &BackendBridge::backendError, this,
                [this](const QString &message) {
                    print(QStringLiteral("[backend] %1").arg(message));
                });
        connect(backend, &BackendBridge::backendMessage, this,
                [this](const QString &text) {
                    print(QStringLiteral("  %1").arg(text));
                });
    }
}

void TestConsole::onReturnPressed() {
    QString cmd = input->text();
    if (cmd.isEmpty()) return;

    output->appendPlainText("> " + cmd);
    processCommand(cmd);
    input->clear();
}

void TestConsole::print(const QString &text) {
    output->appendPlainText(text);
}

QStringList TestConsole::tokenize(const QString &line) {
    QStringList tokens;
    QString current;
    bool inQuotes = false;
    bool hasToken = false;

    for (const QChar &c : line) {
        if (c == QLatin1Char('"')) {
            inQuotes = !inQuotes;
            hasToken = true;          // "" ist ein leeres, aber echtes Token
            continue;
        }
        if (c.isSpace() && !inQuotes) {
            if (hasToken) {
                tokens.append(current);
                current.clear();
                hasToken = false;
            }
            continue;
        }
        current.append(c);
        hasToken = true;
    }
    if (hasToken)
        tokens.append(current);

    return tokens;
}

QString TestConsole::takeOption(QStringList &args, const QString &name,
                                const QString &fallback) {
    const QString prefix = name + QLatin1Char('=');
    for (int i = 0; i < args.size(); ++i) {
        if (args.at(i).startsWith(prefix, Qt::CaseInsensitive)) {
            const QString value = args.at(i).mid(prefix.size());
            args.removeAt(i);
            return value;
        }
    }
    return fallback;
}

QString TestConsole::currentPlatform() {
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("darwin");
#else
    return QStringLiteral("linux");
#endif
}

QString TestConsole::defaultDestination() {
    QString base = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (base.isEmpty())
        base = QDir::currentPath();
    return QDir(base).filePath(QStringLiteral("AstralInstall"));
}

void TestConsole::handleCheck(const QStringList &args) {
    if (args.isEmpty()) {
        print(QStringLiteral("Usage: check <repo-url>\n"));
        return;
    }
    if (!backend) {
        print(QStringLiteral("Keine Backend-Bridge verbunden.\n"));
        return;
    }

    const QString repo = args.first();
    print(QStringLiteral("Frage neueste Version ab: %1 ...").arg(repo));
    backend->checkRelease(repo);
}

void TestConsole::handleDownload(QStringList args) {
    if (args.isEmpty()) {
        print(QStringLiteral(
            "Usage: download <repo-url> [dest=<pfad>] [platform=<hinweis>]\n"
            "  repo      GitHub-URL, z.B. https://github.com/user/projekt\n"
            "  dest      Zielordner (Standard: %1)\n"
            "  platform  Filter auf den Asset-Namen, z.B. windows/linux/darwin\n"
            "            (Standard: %2)\n"
            "Beispiel:\n"
            "  download https://github.com/hannes-swd/code-miner "
            "dest=\"C:\\Games\\code-miner\" platform=windows\n")
                  .arg(defaultDestination(), currentPlatform()));
        return;
    }
    if (!backend) {
        print(QStringLiteral("Keine Backend-Bridge verbunden.\n"));
        return;
    }

    // Erst die benannten Parameter herausziehen, was danach uebrig
    // bleibt, ist die Repo-URL.
    const QString destination = takeOption(args, QStringLiteral("dest"),
                                           defaultDestination());
    const QString platform = takeOption(args, QStringLiteral("platform"),
                                        currentPlatform());

    if (args.isEmpty()) {
        print(QStringLiteral("Fehlt: die Repo-URL.\n"));
        return;
    }
    if (args.size() > 1) {
        print(QStringLiteral("Unbekannte Parameter: %1\n")
                  .arg(args.mid(1).join(QLatin1Char(' '))));
        return;
    }

    const QString repo = args.first();
    print(QStringLiteral("Lade herunter: %1\n  Ziel:     %2\n  Platform: %3")
              .arg(repo, destination, platform));
    backend->downloadNewest(repo, platform, destination);
}

void TestConsole::processCommand(const QString &cmd) {
    QStringList tokens = tokenize(cmd);
    if (tokens.isEmpty())
        return;

    const QString name = tokens.takeFirst().toLower();

    if (name == QLatin1String("help")) {
        print(QStringLiteral(
            "Commands:\n"
            "  check <repo-url>\n"
            "      Neueste Release-Version abfragen.\n"
            "  download <repo-url> [dest=<pfad>] [platform=<hinweis>]\n"
            "      Neuestes Release herunterladen. Pfade mit Leerzeichen\n"
            "      in Anfuehrungszeichen: dest=\"C:\\Program Files\\App\"\n"
            "  clear\n"
            "  exit\n"));
        return;
    }
    if (name == QLatin1String("check")) {
        handleCheck(tokens);
        return;
    }
    if (name == QLatin1String("download")) {
        handleDownload(tokens);
        return;
    }
    if (name == QLatin1String("clear")) {
        output->clear();
        return;
    }
    if (name == QLatin1String("exit")) {
        close();
        return;
    }

    print(QStringLiteral("Unknown command. Type 'help'\n"));
}
