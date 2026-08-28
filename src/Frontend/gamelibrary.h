#ifndef GAMELIBRARY_H
#define GAMELIBRARY_H

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QVector>

class QJsonArray;

/*
 * The list of games shown in Library.qml, exposed to QML as the context
 * property "gameLibrary" (see main.cpp).
 *
 * TEMPORARY SOURCE: right now the entries come from games.json next to
 * the executable. Nothing else in here depends on that - setGames() is
 * the only way data enters the model. Once the backend answers a
 * "list_games" action, connect its signal to setGames() and drop
 * loadFromFile(); the model and the QML stay untouched.
 */
class GameLibrary : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    // Role names as they appear in the QML delegate.
    enum Role {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        DeveloperRole,
        CapsuleRole,
        BannerRole,
        PlatformsRole,
        GenresRole,
        VersionRole,
    };

    explicit GameLibrary(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Replaces the whole list. This is the seam: file today, backend later.
    void setGames(const QJsonArray &games);

    // Reads games.json from disk. Leaves the model empty and warns if the
    // file is missing or malformed - a broken mock file must not take the
    // UI down with it.
    void loadFromFile(const QString &path);

    // Absolute path of games.json, next to the executable. Same convention
    // as BackendBridge::backendExecutablePath().
    static QString defaultFilePath();

public slots:
    // Re-reads the file, so the JSON can be edited while the app runs.
    // Goes away together with loadFromFile().
    void reload();

signals:
    void countChanged();

private:
    struct Game {
        QString name;
        QString description;
        QString developer;
        QString capsule;
        QString banner;
        QStringList platforms;
        QStringList genres;
        QString version;
    };

    QVector<Game> m_games;
    QString m_sourcePath;
};

#endif
