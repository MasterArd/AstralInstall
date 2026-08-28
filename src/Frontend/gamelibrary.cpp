#include "gamelibrary.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace {

// The keys as they are spelled in games.json. Kept together so renaming a
// field in the JSON is a one-line change here.
constexpr auto KeyName = "Game_name";
constexpr auto KeyDescription = "description";
constexpr auto KeyDeveloper = "developer";
constexpr auto KeyBanner = "banner";
constexpr auto KeyPlatforms = "platforms";
constexpr auto KeyGenres = "genres";
constexpr auto KeyVersion = "Game_version";

const QString FileName = QStringLiteral("games.json");

// A version written as a JSON number (1.0 instead of "1.0") would silently
// come out empty, because QJsonValue::toString() only handles strings. Take
// it anyway so the card is not blank, but say so - "1.10" would arrive as
// 1.1 here, which sorts below 1.9.
QString stringField(const QJsonObject &game, const char *key)
{
    const QJsonValue value = game.value(QLatin1String(key));
    if (value.isString())
        return value.toString();
    if (value.isDouble()) {
        const QString text = QString::number(value.toDouble());
        qWarning("[GameLibrary] \"%s\" is a number (%s), expected a string. "
                 "Version numbers lose trailing digits this way.",
                 key, qUtf8Printable(text));
        return text;
    }
    return QString();
}

QStringList stringListField(const QJsonObject &game, const char *key)
{
    QStringList list;
    const QJsonArray array = game.value(QLatin1String(key)).toArray();
    for (const QJsonValue &entry : array) {
        if (entry.isString())
            list.append(entry.toString());
    }
    return list;
}

}

GameLibrary::GameLibrary(QObject *parent)
    : QAbstractListModel(parent)
{
}

int GameLibrary::rowCount(const QModelIndex &parent) const
{
    // A list has no children, only top level rows.
    if (parent.isValid())
        return 0;
    return m_games.size();
}

QVariant GameLibrary::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_games.size())
        return QVariant();

    const Game &game = m_games.at(index.row());
    switch (role) {
    case NameRole:        return game.name;
    case DescriptionRole: return game.description;
    case DeveloperRole:   return game.developer;
    case BannerRole:      return game.banner;
    case PlatformsRole:   return game.platforms;
    case GenresRole:      return game.genres;
    case VersionRole:     return game.version;
    default:              return QVariant();
    }
}

QHash<int, QByteArray> GameLibrary::roleNames() const
{
    return {
        {NameRole,        "name"},
        {DescriptionRole, "description"},
        {DeveloperRole,   "developer"},
        {BannerRole,      "banner"},
        {PlatformsRole,   "platforms"},
        {GenresRole,      "genres"},
        {VersionRole,     "version"},
    };
}

void GameLibrary::setGames(const QJsonArray &games)
{
    beginResetModel();
    m_games.clear();
    m_games.reserve(games.size());

    for (const QJsonValue &entry : games) {
        if (!entry.isObject()) {
            qWarning("[GameLibrary] skipping an entry that is not an object");
            continue;
        }

        const QJsonObject object = entry.toObject();
        Game game;
        game.name = stringField(object, KeyName);
        game.description = stringField(object, KeyDescription);
        game.developer = stringField(object, KeyDeveloper);
        game.banner = stringField(object, KeyBanner);
        game.platforms = stringListField(object, KeyPlatforms);
        game.genres = stringListField(object, KeyGenres);
        game.version = stringField(object, KeyVersion);

        if (game.name.isEmpty())
            qWarning("[GameLibrary] an entry has no \"%s\"", KeyName);

        m_games.append(game);
    }

    endResetModel();
    emit countChanged();
}

QString GameLibrary::defaultFilePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(FileName);
}

void GameLibrary::loadFromFile(const QString &path)
{
    m_sourcePath = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("[GameLibrary] cannot open %s: %s",
                 qUtf8Printable(path), qUtf8Printable(file.errorString()));
        setGames(QJsonArray());
        return;
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning("[GameLibrary] %s is not valid JSON at offset %d: %s",
                 qUtf8Printable(path), parseError.offset,
                 qUtf8Printable(parseError.errorString()));
        setGames(QJsonArray());
        return;
    }

    if (!doc.isArray()) {
        qWarning("[GameLibrary] %s must contain an array of games",
                 qUtf8Printable(path));
        setGames(QJsonArray());
        return;
    }

    setGames(doc.array());
    qInfo("[GameLibrary] loaded %lld game(s) from %s",
          static_cast<long long>(m_games.size()), qUtf8Printable(path));
}

void GameLibrary::reload()
{
    if (m_sourcePath.isEmpty())
        return;
    loadFromFile(m_sourcePath);
}
