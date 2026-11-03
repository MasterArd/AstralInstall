#ifndef INSTALLMANAGER_H
#define INSTALLMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVariantList>

class BackendBridge;

/*
 * Install / update / remove games. In qml its "installManager".
 * Backend only downloads, all the local stuff is here.
 *
 * release zip:
 *   game/           exe + code        -> replaced on update
 *   assets/                           -> replaced on update
 *   saves/          default saves     -> never ovetwritten
 *   AstralInstall/  Banner.png etc    -> replaced on update
 *
 * on disk: <gamesRoot>/<Game>/game, assets, saves, AstralInstall, astral.json
 * no astral.json = not installed (its wtitten as the last thimg)
 */
class InstallManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList installedGames READ installedGames NOTIFY installedGamesChanged)
    Q_PROPERTY(QStringList installedNames READ installedNames NOTIFY installedGamesChanged)
    Q_PROPERTY(QStringList busyGames READ busyGames NOTIFY busyGamesChanged)

public:
    explicit InstallManager(BackendBridge *backend, QObject *parent = nullptr);

    QVariantList installedGames() const { return m_installed; }
    QStringList installedNames() const;
    QStringList busyGames() const { return m_jobs.keys(); }

    static QString currentPlatform();
    static QString folderName(const QString &gameName);

public slots:
    void install(const QString &name, const QString &repo, const QString &banner = QString());
    void update(const QString &name); // keeps saves
    void checkForUpdate(const QString &name);
    void installFromArchive(const QString &name, const QString &zipPath, const QString &version = QString(),
                            const QString &repo = QString(), const QString &banner = QString());
    bool uninstall(const QString &name, bool keepSaves = true);

    bool isInstalled(const QString &name) const;
    QString installedVersion(const QString &name) const;
    QString gameDir(const QString &name) const;

signals:
    void installStarted(const QString &name);
    void installFinished(const QString &name);
    void updateFinished(const QString &name);
    void updateAvailable(const QString &name, const QString &version);
    void upToDate(const QString &name);
    void installFailed(const QString &name, const QString &error);
    void statusChanged(const QString &name, const QString &status);
    void installedGamesChanged();
    void busyGamesChanged();

private:
    enum class Mode { Install, Update, Check };
    enum class Stage { Checking, Downloading, Listing, Extracting };

    struct Job {
        Mode mode = Mode::Install;
        Stage stage = Stage::Checking;
        QString name;
        QString repo;
        QString banner;
        QString version;
        QString zipPath;
        QPointer<QProcess> process;
    };

    // runs in the staging folder with only ascii names (archive.zip, extracted)
    // becsuse windows tar.exe cant handle umlauts in the arguments
    struct ArchiveTool {
        QString program;
        QStringList listArgs;
        QStringList extractArgs;
        QProcessEnvironment environment;
        QString workingDirectory;
    };

    void onReleaseChecked(const QString &repo, const QString &version);
    void onDownloadFinished(const QString &repo, const QString &destination);
    void onRequestFailed(const QString &repo, const QString &error);

    bool startJob(const Job &job);
    Job *jobForRepo(const QString &repo, Stage stage);
    void startDownload(Job &job);
    void startListing(Job &job);
    void startExtracting(Job &job);
    void applyExtracted(const QString &name);
    void finishJob(const QString &name);
    void failJob(const QString &name, const QString &error);

    QString jobStagingDir(const QString &name) const;
    QString extractDir(const QString &name) const;
    QString archivePath(const QString &name) const;

    bool writeManifest(const Job &job, QString *error) const;
    QVariantMap readManifest(const QString &gameFolder) const;
    void refreshInstalled();
    void recoverInterruptedUpdates();

    static ArchiveTool findArchiveTool(const QString &stagingDir);
    static bool isUnsafeEntry(const QString &entry);
    static bool staysInside(const QString &root, QString *offender);
    static QString findContentRoot(const QString &extracted);
    static bool moveDir(const QString &from, const QString &to);
    static bool mergeMissing(const QString &from, const QString &to, QString *error);

    BackendBridge *m_backend;
    QHash<QString, Job> m_jobs;
    QVariantList m_installed;
};

#endif
