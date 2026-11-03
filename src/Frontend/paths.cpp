#include "paths.h"

#include <QDir>
#include <QStandardPaths>

namespace {

const QString AppFolder = "AstralInstall";
QString gamesRootOverride; // from settings.json, empty = default

QString ensured(const QString &dir)
{
    QDir().mkpath(dir);
    return dir;
}

}

QString Paths::dataDir()
{
    // not AppLocalDataLocation, that would be AstralInstall\AstralInstall
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    return ensured(QDir(base).filePath(AppFolder));
}

QString Paths::gamesRoot()
{
    if (!gamesRootOverride.isEmpty())
        return ensured(gamesRootOverride);
    return ensured(QDir(dataDir()).filePath("games"));
}

void Paths::setGamesRoot(const QString &dir)
{
    if (dir.isEmpty()) {
        gamesRootOverride.clear();
        return;
    }
    if (QDir::isRelativePath(dir)) {
        qWarning("[Paths] games root \"%s\" is not an absolute path, ignoring it", qUtf8Printable(dir));
        return;
    }
    gamesRootOverride = QDir::cleanPath(dir);
}

QString Paths::stagingDir()
{
    return ensured(QDir(gamesRoot()).filePath(".staging"));
}

QString Paths::logDir()
{
    return ensured(QDir(dataDir()).filePath("logs"));
}

QString Paths::configFile()
{
    // same place like before so old configs still wotk
    QString dir = ensured(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    return QDir(dir).filePath("settings.json");
}

QString Paths::lockFile()
{
    return QDir(dataDir()).filePath("AstralInstall.lock");
}
