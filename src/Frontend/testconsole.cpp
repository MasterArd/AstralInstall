#include "testconsole.h"

#include <QVBoxLayout>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>

#include "backendbridge.h"
#include "installmanager.h"
#include "paths.h"

TestConsole::TestConsole(BackendBridge *bridge, InstallManager *installManager,
                         QWidget *parent)
    : QDialog(parent), backend(bridge), installer(installManager)
{
    setWindowTitle("Test Console");
    setGeometry(100, 100, 600, 400);

    // Output area
    output = new QPlainTextEdit();
    output->setReadOnly(true);
    // Do not hard-wire "Courier" - that name often does not exist on
    // Linux. Qt returns the monospace font of the running system here.
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

    // The backend answers asynchronously. The responses end up here in
    // the console so the result of a command is actually visible.
    if (backend) {
        connect(backend, &BackendBridge::releaseChecked, this,
                [this](const QString &repo, const QString &version) {
                    print(QStringLiteral("[ok] %1 -> %2").arg(repo, version));
                });
        connect(backend, &BackendBridge::downloadFinished, this,
                [this](const QString &repo, const QString &destination) {
                    print(QStringLiteral("[ok] %1 downloaded to %2")
                              .arg(repo, destination));
                });
        connect(backend, &BackendBridge::requestFailed, this,
                [this](const QString &repo, const QString &error) {
                    print(QStringLiteral("[error] %1: %2").arg(repo, error));
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

    if (installer) {
        connect(installer, &InstallManager::statusChanged, this,
                [this](const QString &name, const QString &status) {
                    print(QStringLiteral("  %1: %2").arg(name, status));
                });
        connect(installer, &InstallManager::installFinished, this,
                [this](const QString &name) {
                    print(QStringLiteral("[ok] %1 installed to %2")
                              .arg(name, installer->gameDir(name)));
                });
        connect(installer, &InstallManager::updateFinished, this,
                [this](const QString &name) {
                    print(QStringLiteral("[ok] %1 updated to %2")
                              .arg(name, installer->installedVersion(name)));
                });
        connect(installer, &InstallManager::updateAvailable, this,
                [this](const QString &name, const QString &version) {
                    print(QStringLiteral("[ok] %1: update available (%2 -> %3)")
                              .arg(name, installer->installedVersion(name), version));
                });
        connect(installer, &InstallManager::upToDate, this,
                [this](const QString &name) {
                    print(QStringLiteral("[ok] %1 is up to date").arg(name));
                });
        connect(installer, &InstallManager::installFailed, this,
                [this](const QString &name, const QString &error) {
                    print(QStringLiteral("[error] %1: %2").arg(name, error));
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
            hasToken = true;          // "" is an empty but real token
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
    return InstallManager::currentPlatform();
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
        print(QStringLiteral("No backend bridge connected.\n"));
        return;
    }

    const QString repo = args.first();
    print(QStringLiteral("Checking newest version: %1 ...").arg(repo));
    backend->checkRelease(repo);
}

void TestConsole::handleDownload(QStringList args) {
    if (args.isEmpty()) {
        print(QStringLiteral(
            "Usage: download <repo-url> [dest=<path>] [platform=<hint>]\n"
            "  repo      GitHub URL, e.g. https://github.com/user/project\n"
            "  dest      Target folder (default: %1)\n"
            "  platform  Filter on the asset name, e.g. windows/linux/darwin\n"
            "            (default: %2)\n"
            "Example:\n"
            "  download https://github.com/hannes-swd/code-miner "
            "dest=\"C:\\Games\\code-miner\" platform=windows\n")
                  .arg(defaultDestination(), currentPlatform()));
        return;
    }
    if (!backend) {
        print(QStringLiteral("No backend bridge connected.\n"));
        return;
    }

    // Pull out the named parameters first; whatever is left over is the
    // repo URL.
    const QString destination = takeOption(args, QStringLiteral("dest"),
                                           defaultDestination());
    const QString platform = takeOption(args, QStringLiteral("platform"),
                                        currentPlatform());

    if (args.isEmpty()) {
        print(QStringLiteral("Missing: the repo URL.\n"));
        return;
    }
    if (args.size() > 1) {
        print(QStringLiteral("Unknown parameters: %1\n")
                  .arg(args.mid(1).join(QLatin1Char(' '))));
        return;
    }

    const QString repo = args.first();
    print(QStringLiteral("Downloading: %1\n  Target:   %2\n  Platform: %3")
              .arg(repo, destination, platform));
    backend->downloadNewest(repo, platform, destination);
}

void TestConsole::handleInstall(const QStringList &args) {
    if (args.size() != 2) {
        print(QStringLiteral("Usage: install <name> <repo-url>\n"
                             "  Quote names with spaces: install \"My Game\" https://...\n"));
        return;
    }
    if (!installer) {
        print(QStringLiteral("No install manager connected.\n"));
        return;
    }

    print(QStringLiteral("Installing %1 from %2 ...").arg(args.at(0), args.at(1)));
    installer->install(args.at(0), args.at(1));
}

void TestConsole::handleInstallZip(QStringList args) {
    const QString version = takeOption(args, QStringLiteral("version"));
    if (args.size() != 2) {
        print(QStringLiteral("Usage: installzip <name> <path.zip> [version=<v>]\n"
                             "  Installs from a local zip, without GitHub.\n"));
        return;
    }
    if (!installer) {
        print(QStringLiteral("No install manager connected.\n"));
        return;
    }

    print(QStringLiteral("Installing %1 from %2 ...").arg(args.at(0), args.at(1)));
    installer->installFromArchive(args.at(0), args.at(1), version);
}

void TestConsole::handleGameCommand(const QString &command, QStringList args) {
    const QString keepSaves = takeOption(args, QStringLiteral("keepsaves"),
                                         QStringLiteral("yes"));
    if (args.size() != 1) {
        print(QStringLiteral("Usage: %1 <name>%2\n")
                  .arg(command, command == QLatin1String("uninstall")
                                    ? QStringLiteral(" [keepsaves=yes|no]")
                                    : QString()));
        return;
    }
    if (!installer) {
        print(QStringLiteral("No install manager connected.\n"));
        return;
    }

    const QString name = args.first();
    if (command == QLatin1String("update")) {
        print(QStringLiteral("Updating %1 ...").arg(name));
        installer->update(name);
    } else if (command == QLatin1String("checkupdate")) {
        print(QStringLiteral("Checking %1 for updates ...").arg(name));
        installer->checkForUpdate(name);
    } else if (command == QLatin1String("uninstall")) {
        const bool keep = keepSaves.compare(QLatin1String("no"), Qt::CaseInsensitive) != 0;
        if (installer->uninstall(name, keep))
            print(QStringLiteral("[ok] %1 removed%2").arg(name, keep ? QStringLiteral(", saves kept")
                                                                     : QString()));
        else
            print(QStringLiteral("[error] %1 is not installed or could not be removed").arg(name));
    }
}

void TestConsole::printGames() {
    print(QStringLiteral("Data:  %1\nGames: %2")
              .arg(QDir::toNativeSeparators(Paths::dataDir()),
                   QDir::toNativeSeparators(Paths::gamesRoot())));
    if (!installer)
        return;

    const QVariantList games = installer->installedGames();
    if (games.isEmpty())
        print(QStringLiteral("  (no games installed)"));
    for (const QVariant &entry : games) {
        const QVariantMap game = entry.toMap();
        print(QStringLiteral("  %1  %2  %3")
                  .arg(game.value(QStringLiteral("name")).toString(),
                       game.value(QStringLiteral("version")).toString(),
                       game.value(QStringLiteral("path")).toString()));
    }
    print(QString());
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
            "      Query the newest release version.\n"
            "  download <repo-url> [dest=<path>] [platform=<hint>]\n"
            "      Download the newest release. Quote paths that contain\n"
            "      spaces: dest=\"C:\\Program Files\\App\"\n"
            "  install <name> <repo-url>\n"
            "      Download and install the newest release zip.\n"
            "  installzip <name> <path.zip> [version=<v>]\n"
            "      Install from a local zip.\n"
            "  update <name>      checkupdate <name>\n"
            "  uninstall <name> [keepsaves=yes|no]\n"
            "  games\n"
            "      List installed games and the data folders.\n"
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
    if (name == QLatin1String("install")) {
        handleInstall(tokens);
        return;
    }
    if (name == QLatin1String("installzip")) {
        handleInstallZip(tokens);
        return;
    }
    if (name == QLatin1String("update") || name == QLatin1String("checkupdate")
        || name == QLatin1String("uninstall")) {
        handleGameCommand(name, tokens);
        return;
    }
    if (name == QLatin1String("games")) {
        printGames();
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
