#include "installmanager.h"
#include "backendbridge.h"
#include "paths.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUrl>
#include <algorithm>

namespace {

const QString GamePart = "game";
const QString AssetsPart = "assets";
const QString MetaPart = "AstralInstall"; // banner, config.json usw
const QString SavesPart = "saves";
const QString LocalBanner = "Banner.png";
const QString ManifestName = "astral.json";
const QString ArchiveName = "archive.zip";
const QString ExtractName = "extracted";
const QString NewSuffix = ".new";
const QString OldSuffix = ".old";

// saves is not in hrre on purpose!!
const QStringList ReplacedParts{GamePart, AssetsPart, MetaPart};

constexpr int ManifestVersion = 1;
constexpr qint64 SpaceFactor = 3; // zip + unpacked + old game

#ifdef Q_OS_WIN
constexpr Qt::CaseSensitivity PathCase = Qt::CaseInsensitive;
#else
constexpr Qt::CaseSensitivity PathCase = Qt::CaseSensitive;
#endif

bool removePath(const QString &path)
{
    QFileInfo info(path);
    if (info.isDir() && !info.isSymLink())
        return QDir(path).removeRecursively();
    if (info.exists() || info.isSymLink())
        return QFile::remove(path);
    return true;
}

bool copyRecursively(const QString &from, const QString &to)
{
    if (!QDir().mkpath(to))
        return false;

    auto entries = QDir(from).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        QString target = QDir(to).filePath(entry.fileName());
        if (entry.isDir() && !entry.isSymLink()) {
            if (!copyRecursively(entry.filePath(), target))
                return false;
        } else if (!QFile::copy(entry.filePath(), target)) {
            return false;
        }
    }
    return true;
}

QString processOutput(QProcess *process)
{
    QString err = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
    if (!err.isEmpty())
        return err;
    return QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
}

}

InstallManager::InstallManager(BackendBridge *backend, QObject *parent)
    : QObject(parent), m_backend(backend)
{
    if (m_backend) {
        connect(m_backend, &BackendBridge::releaseChecked, this, &InstallManager::onReleaseChecked);
        connect(m_backend, &BackendBridge::downloadFinished, this, &InstallManager::onDownloadFinished);
        connect(m_backend, &BackendBridge::requestFailed, this, &InstallManager::onRequestFailed);
    }

    recoverInterruptedUpdates();
    refreshInstalled();
}

QStringList InstallManager::installedNames() const
{
    QStringList names;
    for (const QVariant &game : m_installed)
        names << game.toMap().value("name").toString();
    return names;
}

QString InstallManager::currentPlatform()
{
#if defined(Q_OS_WIN)
    return "windows";
#elif defined(Q_OS_MACOS)
    return "darwin";
#else
    return "linux";
#endif
}

QString InstallManager::folderName(const QString &gameName)
{
    // windows rules for evetywhere, so the folder name is same on linux
    static const QString forbidden = "<>:\"/\\|?*";
    QString out;
    for (const QChar c : gameName.trimmed())
        out.append(c.unicode() < 32 || forbidden.contains(c) ? QChar('_') : c);

    while (out.endsWith('.') || out.endsWith(' '))
        out.chop(1);

    if (out.isEmpty() || out.startsWith('.')) // .staging is ours
        out.prepend('_');

    static const QStringList reserved{
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};
    if (reserved.contains(out.section('.', 0, 0).toUpper()))
        out.prepend('_');

    return out;
}

void InstallManager::install(const QString &name, const QString &repo, const QString &banner)
{
    if (name.trimmed().isEmpty() || repo.trimmed().isEmpty()) {
        emit installFailed(name, tr("Game name and repository are required"));
        return;
    }

    Job job;
    job.mode = Mode::Install;
    job.name = name;
    job.repo = repo;
    job.banner = banner;
    if (!startJob(job))
        return;

    emit statusChanged(name, tr("Checking version..."));
    m_backend->checkRelease(repo);
}

void InstallManager::update(const QString &name)
{
    if (!isInstalled(name)) {
        emit installFailed(name, tr("%1 is not installed").arg(name));
        return;
    }

    QVariantMap manifest = readManifest(QDir(Paths::gamesRoot()).filePath(folderName(name)));
    Job job;
    job.mode = Mode::Update;
    job.name = name;
    job.repo = manifest.value("repo").toString();
    job.banner = manifest.value("banner").toString();
    if (job.repo.isEmpty()) {
        emit installFailed(name, tr("No repository stored for %1, cannot update").arg(name));
        return;
    }
    if (!startJob(job))
        return;

    emit statusChanged(name, tr("Checking version..."));
    m_backend->checkRelease(job.repo);
}

void InstallManager::checkForUpdate(const QString &name)
{
    if (!isInstalled(name))
        return;

    QVariantMap manifest = readManifest(QDir(Paths::gamesRoot()).filePath(folderName(name)));
    Job job;
    job.mode = Mode::Check;
    job.name = name;
    job.repo = manifest.value("repo").toString();
    if (job.repo.isEmpty())
        return; // was from local zip, nothinf to compare
    if (!startJob(job))
        return;

    m_backend->checkRelease(job.repo);
}

void InstallManager::installFromArchive(const QString &name, const QString &zipPath,
                                        const QString &version, const QString &repo,
                                        const QString &banner)
{
    if (name.trimmed().isEmpty()) {
        emit installFailed(name, tr("No game name given"));
        return;
    }
    if (!QFileInfo(zipPath).isFile()) {
        emit installFailed(name, tr("Archive not found: %1").arg(zipPath));
        return;
    }

    Job job;
    job.mode = isInstalled(name) ? Mode::Update : Mode::Install;
    job.name = name;
    job.repo = repo;
    job.banner = banner;
    job.version = version;
    job.zipPath = QFileInfo(zipPath).absoluteFilePath();
    job.stage = Stage::Listing; // file is alteady here, no backemd
    if (!startJob(job))
        return;

    startListing(m_jobs[name]);
}

bool InstallManager::uninstall(const QString &name, bool keepSaves)
{
    if (m_jobs.contains(name)) {
        emit installFailed(name, tr("%1 is busy, try again when it is done").arg(name));
        return false;
    }

    QString folder = QDir(Paths::gamesRoot()).filePath(folderName(name));
    if (!QFileInfo(folder).isDir())
        return false;

    bool ok = true;
    if (keepSaves) {
        // manifest first, so if something fail the game is not "installed" anymore
        ok &= removePath(QDir(folder).filePath(ManifestName));
        for (const QString &part : ReplacedParts) {
            ok &= removePath(QDir(folder).filePath(part));
            ok &= removePath(QDir(folder).filePath(part + NewSuffix));
            ok &= removePath(QDir(folder).filePath(part + OldSuffix));
        }
        QDir(folder).rmdir(QDir(folder).filePath(SavesPart)); // only if empty
        QDir().rmdir(folder);
    } else {
        ok = QDir(folder).removeRecursively();
    }

    if (!ok)
        emit installFailed(name, tr("Some files of %1 could not be removed. Close the game and try again.").arg(name));

    refreshInstalled();
    return ok;
}

bool InstallManager::isInstalled(const QString &name) const
{
    QString folder = QDir(Paths::gamesRoot()).filePath(folderName(name));
    return QFileInfo::exists(QDir(folder).filePath(ManifestName));
}

QString InstallManager::installedVersion(const QString &name) const
{
    if (!isInstalled(name))
        return {};
    return readManifest(QDir(Paths::gamesRoot()).filePath(folderName(name))).value("version").toString();
}

QString InstallManager::gameDir(const QString &name) const
{
    if (!isInstalled(name))
        return {};
    return QDir::toNativeSeparators(QDir(Paths::gamesRoot()).filePath(folderName(name)));
}

// check version -> download -> list -> unpack -> apply

bool InstallManager::startJob(const Job &job)
{
    if (m_jobs.contains(job.name)) {
        qWarning("[Install] %s is already busy", qUtf8Printable(job.name));
        return false;
    }
    if (job.zipPath.isEmpty() && !m_backend) {
        emit installFailed(job.name, tr("No backend connected"));
        return false;
    }

    m_jobs.insert(job.name, job);
    emit busyGamesChanged();
    if (job.mode != Mode::Check)
        emit installStarted(job.name);
    return true;
}

InstallManager::Job *InstallManager::jobForRepo(const QString &repo, Stage stage)
{
    for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
        if (it->repo == repo && it->stage == stage)
            return &it.value();
    }
    return nullptr;
}

void InstallManager::onReleaseChecked(const QString &repo, const QString &version)
{
    Job *job = jobForRepo(repo, Stage::Checking);
    if (!job)
        return;

    job->version = version;
    QString name = job->name;
    QString installed = installedVersion(name);

    if (job->mode == Mode::Check) {
        m_jobs.remove(name);
        emit busyGamesChanged();
        if (!version.isEmpty() && version != installed)
            emit updateAvailable(name, version);
        else
            emit upToDate(name);
        return;
    }

    if (job->mode == Mode::Update && !version.isEmpty() && version == installed) {
        m_jobs.remove(name);
        emit busyGamesChanged();
        emit upToDate(name);
        return;
    }

    startDownload(*job);
}

void InstallManager::startDownload(Job &job)
{
    job.stage = Stage::Downloading;

    QString dir = QDir(jobStagingDir(job.name)).filePath("download");
    removePath(dir);
    QDir().mkpath(dir);

    emit statusChanged(job.name, tr("Downloading..."));
    m_backend->downloadNewest(job.repo, currentPlatform(), QDir::toNativeSeparators(dir));
}

void InstallManager::onDownloadFinished(const QString &repo, const QString &destination)
{
    Q_UNUSED(destination)
    Job *job = jobForRepo(repo, Stage::Downloading);
    if (!job)
        return;

    QDir dir(QDir(jobStagingDir(job->name)).filePath("download"));
    auto zips = dir.entryInfoList({"*.zip"}, QDir::Files);
    if (zips.isEmpty()) {
        QStringList other = dir.entryList(QDir::Files);
        failJob(job->name, tr("The release asset is not a zip (%1). Games must ship one zip with game/, assets/, saves/ and AstralInstall/.")
                               .arg(other.isEmpty() ? tr("nothing downloaded") : other.join(", ")));
        return;
    }

    job->zipPath = zips.first().absoluteFilePath();
    startListing(*job);
}

void InstallManager::onRequestFailed(const QString &repo, const QString &error)
{
    Job *job = jobForRepo(repo, Stage::Checking);
    if (!job)
        job = jobForRepo(repo, Stage::Downloading);
    if (job)
        failJob(job->name, error);
}

void InstallManager::startListing(Job &job)
{
    job.stage = Stage::Listing;
    QString name = job.name;

    QString extracted = extractDir(name);
    removePath(extracted);
    if (!QDir().mkpath(extracted)) {
        failJob(name, tr("Cannot create %1").arg(QDir::toNativeSeparators(extracted)));
        return;
    }

    qint64 needed = QFileInfo(job.zipPath).size() * SpaceFactor;
    QStorageInfo storage(Paths::gamesRoot());
    if (storage.isValid() && storage.bytesAvailable() >= 0 && storage.bytesAvailable() < needed) {
        failJob(name, tr("Not enough disk space: %1 MB needed, %2 MB free")
                          .arg(needed / (1024 * 1024))
                          .arg(storage.bytesAvailable() / (1024 * 1024)));
        return;
    }

    // doenload is alteady in staging so just rename, a local zip gets copyed
    // so the users file stay how it is
    QString archive = archivePath(name);
    removePath(archive);
    bool inStaging = job.zipPath.startsWith(jobStagingDir(name) + '/', PathCase);
    if (!(inStaging ? QFile::rename(job.zipPath, archive) : QFile::copy(job.zipPath, archive))) {
        failJob(name, tr("Cannot copy %1 to the staging folder").arg(QDir::toNativeSeparators(job.zipPath)));
        return;
    }
    job.zipPath = archive;

    ArchiveTool tool = findArchiveTool(jobStagingDir(name));
    if (tool.program.isEmpty()) {
#ifdef Q_OS_WIN
        failJob(name, tr("No tool to unpack zip files found (tar.exe or PowerShell)"));
#else
        failJob(name, tr("No tool to unpack zip files found. Please install \"unzip\"."));
#endif
        return;
    }

    emit statusChanged(name, tr("Checking archive..."));

    // first only list, so sometging like ../../evil.dll gets stopped bevore it is written
    auto *process = new QProcess(this);
    process->setProcessEnvironment(tool.environment);
    process->setWorkingDirectory(tool.workingDirectory);
    m_jobs[name].process = process;

    connect(process, &QProcess::errorOccurred, this, [this, name, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        process->deleteLater();
        failJob(name, tr("Cannot start %1: %2").arg(process->program(), process->errorString()));
    });
    connect(process, &QProcess::finished, this, [this, name, process](int exitCode, QProcess::ExitStatus status) {
        process->deleteLater();
        if (!m_jobs.contains(name))
            return;
        if (status != QProcess::NormalExit || exitCode != 0) {
            failJob(name, tr("The zip file is damaged or unreadable: %1").arg(processOutput(process)));
            return;
        }

        auto entries = QString::fromUtf8(process->readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
        for (const QString &entry : entries) {
            if (isUnsafeEntry(entry)) {
                failJob(name, tr("The zip contains an unsafe path and was rejected: %1").arg(entry.trimmed()));
                return;
            }
        }
        startExtracting(m_jobs[name]);
    });

    process->start(tool.program, tool.listArgs);
}

void InstallManager::startExtracting(Job &job)
{
    job.stage = Stage::Extracting;
    QString name = job.name;
    ArchiveTool tool = findArchiveTool(jobStagingDir(name));

    emit statusChanged(name, tr("Unpacking..."));

    auto *process = new QProcess(this);
    process->setProcessEnvironment(tool.environment);
    process->setWorkingDirectory(tool.workingDirectory);
    job.process = process;

    connect(process, &QProcess::errorOccurred, this, [this, name, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        process->deleteLater();
        failJob(name, tr("Cannot start %1: %2").arg(process->program(), process->errorString()));
    });
    connect(process, &QProcess::finished, this, [this, name, process](int exitCode, QProcess::ExitStatus status) {
        process->deleteLater();
        if (!m_jobs.contains(name))
            return;
        if (status != QProcess::NormalExit || exitCode != 0) {
            failJob(name, tr("Unpacking failed: %1").arg(processOutput(process)));
            return;
        }
        applyExtracted(name);
    });

    process->start(tool.program, tool.extractArgs);
}

void InstallManager::applyExtracted(const QString &name)
{
    if (!m_jobs.contains(name))
        return;

    emit statusChanged(name, tr("Installing..."));

    QString extracted = extractDir(name);
    QString offender;
    if (!staysInside(extracted, &offender)) {
        failJob(name, tr("The zip contains a link that points outside the game folder: %1").arg(offender));
        return;
    }

    QString root = findContentRoot(extracted);
    if (root.isEmpty()) {
        failJob(name, tr("The zip has no game/ folder. Expected game/, assets/, saves/ and AstralInstall/ at the top level."));
        return;
    }

    QDir target(QDir(Paths::gamesRoot()).filePath(folderName(name)));
    if (!QDir().mkpath(target.path())) {
        failJob(name, tr("Cannot create %1").arg(QDir::toNativeSeparators(target.path())));
        return;
    }

    QStringList parts;
    for (const QString &part : ReplacedParts) {
        if (QFileInfo(QDir(root).filePath(part)).isDir())
            parts << part;
    }

    auto at = [&target](const QString &part, const QString &suffix = QString()) {
        return target.filePath(part + suffix);
    };
    auto dropNew = [&]() {
        for (const QString &part : parts)
            removePath(at(part, NewSuffix));
    };

    // 1) new folders next to the old ones (.new), same drive so its just a rename
    for (const QString &part : parts) {
        removePath(at(part, NewSuffix));
        if (!moveDir(QDir(root).filePath(part), at(part, NewSuffix))) {
            dropNew();
            failJob(name, tr("Cannot move the new %1/ folder into place").arg(part));
            return;
        }
    }

    // 2) old ones to .old - this fails when the game is still rumning or antivirus has a file
    QStringList movedAside;
    for (const QString &part : parts) {
        if (!QFileInfo::exists(at(part)))
            continue;
        removePath(at(part, OldSuffix));
        if (!QDir().rename(at(part), at(part, OldSuffix))) {
            for (const QString &done : movedAside)
                QDir().rename(at(done, OldSuffix), at(done));
            dropNew();
            failJob(name, tr("Files of %1 are in use. Close the game and try again.").arg(name));
            return;
        }
        movedAside << part;
    }

    // 3) new ones in
    QStringList placed;
    for (const QString &part : parts) {
        if (!QDir().rename(at(part, NewSuffix), at(part))) {
            for (const QString &done : placed)
                removePath(at(done));
            for (const QString &done : movedAside)
                QDir().rename(at(done, OldSuffix), at(done));
            dropNew();
            failJob(name, tr("Cannot put the new %1/ folder in place, the old version was restored").arg(part));
            return;
        }
        placed << part;
    }

    // 4) delete old. if this fail its cleaned on next start
    for (const QString &part : movedAside)
        removePath(at(part, OldSuffix));

    // saves: only copy files that not exist, players progress is never touvhed
    QString error;
    QString newSaves = QDir(root).filePath(SavesPart);
    if (QFileInfo(newSaves).isDir()) {
        if (!mergeMissing(newSaves, at(SavesPart), &error)) {
            failJob(name, tr("Cannot copy save files: %1").arg(error));
            return;
        }
    } else {
        QDir().mkpath(at(SavesPart));
    }

    if (!writeManifest(m_jobs.value(name), &error)) {
        failJob(name, tr("Cannot write %1: %2").arg(ManifestName, error));
        return;
    }

    finishJob(name);
}

void InstallManager::finishJob(const QString &name)
{
    Job job = m_jobs.take(name);
    removePath(jobStagingDir(name));
    refreshInstalled();
    emit busyGamesChanged();

    qInfo("[Install] %s %s (%s)", qUtf8Printable(name),
          job.mode == Mode::Update ? "updated" : "installed",
          qUtf8Printable(job.version.isEmpty() ? QString("no version") : job.version));

    if (job.mode == Mode::Update)
        emit updateFinished(name);
    else
        emit installFinished(name);
}

void InstallManager::failJob(const QString &name, const QString &error)
{
    if (!m_jobs.contains(name))
        return;

    Job job = m_jobs.take(name);
    if (job.process && job.process->state() != QProcess::NotRunning)
        job.process->kill();
    removePath(jobStagingDir(name));
    emit busyGamesChanged();

    qWarning("[Install] %s failed: %s", qUtf8Printable(name), qUtf8Printable(error));
    emit installFailed(name, error);
}

QString InstallManager::jobStagingDir(const QString &name) const
{
    return QDir(Paths::stagingDir()).filePath(folderName(name));
}

QString InstallManager::extractDir(const QString &name) const
{
    return QDir(jobStagingDir(name)).filePath(ExtractName);
}

QString InstallManager::archivePath(const QString &name) const
{
    return QDir(jobStagingDir(name)).filePath(ArchiveName);
}

bool InstallManager::writeManifest(const Job &job, QString *error) const
{
    QString folder = QDir(Paths::gamesRoot()).filePath(folderName(job.name));

    // local zip doesnt kmow repo/banner so keep the old ones
    QVariantMap previous = readManifest(folder);
    auto pick = [&previous](const QString &value, const char *key) {
        return value.isEmpty() ? previous.value(key).toString() : value;
    };

    QJsonObject manifest;
    manifest["manifestVersion"] = ManifestVersion;
    manifest["name"] = job.name;
    manifest["repo"] = pick(job.repo, "repo");
    manifest["banner"] = pick(job.banner, "banner");
    manifest["version"] = job.version;
    manifest["platform"] = currentPlatform();
    manifest["installedAt"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // QSaveFile so a crash never leave a half file
    QSaveFile file(QDir(folder).filePath(ManifestName));
    if (!file.open(QIODevice::WriteOnly)) {
        *error = file.errorString();
        return false;
    }
    file.write(QJsonDocument(manifest).toJson());
    if (!file.commit()) {
        *error = file.errorString();
        return false;
    }
    return true;
}

QVariantMap InstallManager::readManifest(const QString &gameFolder) const
{
    QFile file(QDir(gameFolder).filePath(ManifestName));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        qWarning("[Install] %s is not valid JSON", qUtf8Printable(file.fileName()));
        return {};
    }

    QVariantMap map = doc.object().toVariantMap();
    QFileInfo folder(gameFolder);
    map["folder"] = folder.fileName();
    map["path"] = QDir::toNativeSeparators(folder.absoluteFilePath());

    // banner from the release is better then the url (works offline)
    QString banner = QDir(gameFolder).filePath(MetaPart + '/' + LocalBanner);
    if (QFileInfo::exists(banner))
        map["banner"] = QUrl::fromLocalFile(banner).toString();

    if (map.value("name").toString().isEmpty())
        map["name"] = folder.fileName();
    return map;
}

void InstallManager::refreshInstalled()
{
    QVariantList games;
    auto folders = QDir(Paths::gamesRoot()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo &folder : folders) {
        if (folder.fileName().startsWith('.'))
            continue;
        if (!QFileInfo::exists(QDir(folder.filePath()).filePath(ManifestName)))
            continue;
        games << readManifest(folder.filePath());
    }

    m_installed = games;
    emit installedGamesChanged();
}

void InstallManager::recoverInterruptedUpdates()
{
    QDir(Paths::stagingDir()).removeRecursively();

    auto folders = QDir(Paths::gamesRoot()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &folder : folders) {
        if (folder.fileName().startsWith('.'))
            continue;

        QDir dir(folder.filePath());
        // if there is a .new left step 3 was not done -> roll back everyrhing,
        // else we could have new game/ with old assets/
        bool unfinished = false;
        for (const QString &part : ReplacedParts)
            unfinished |= QFileInfo::exists(dir.filePath(part + NewSuffix));

        for (const QString &part : ReplacedParts) {
            QString current = dir.filePath(part);
            QString old = dir.filePath(part + OldSuffix);

            if (QFileInfo::exists(old)) {
                if (unfinished || !QFileInfo::exists(current)) {
                    removePath(current);
                    QDir().rename(old, current);
                    qWarning("[Install] restored %s after an interrupted update",
                             qUtf8Printable(QDir::toNativeSeparators(current)));
                } else {
                    removePath(old);
                }
            }
            removePath(dir.filePath(part + NewSuffix));
        }
    }

    Paths::stagingDir(); // makes the folder again
}

InstallManager::ArchiveTool InstallManager::findArchiveTool(const QString &stagingDir)
{
    ArchiveTool tool;
    tool.environment = QProcessEnvironment::systemEnvironment();
    tool.workingDirectory = stagingDir;
    QString zip = ArchiveName;
    QString dest = ExtractName;

#ifdef Q_OS_WIN
    // tar from System32 with full path. git tar in PATH is gnu tar and cant do zip
    QString systemTar = QDir(qEnvironmentVariable("SystemRoot", "C:\\Windows")).filePath("System32/tar.exe");
    if (QFileInfo::exists(systemTar)) {
        tool.program = systemTar;
        tool.listArgs = {"-tf", zip};
        tool.extractArgs = {"-xf", zip, "-C", dest};
        return tool;
    }

    // old windows: powershell. paths go over env vars so no quoting problems
    QString powershell = QStandardPaths::findExecutable("powershell");
    if (!powershell.isEmpty()) {
        tool.program = powershell;
        tool.environment.insert("ASTRAL_ZIP", QDir::toNativeSeparators(QDir(stagingDir).filePath(zip)));
        tool.environment.insert("ASTRAL_DEST", QDir::toNativeSeparators(QDir(stagingDir).filePath(dest)));
        QStringList base{"-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-Command"};
        tool.listArgs = base + QStringList{
            "$ErrorActionPreference='Stop';"
            "[Console]::OutputEncoding=[Text.Encoding]::UTF8;"
            "Add-Type -AssemblyName System.IO.Compression.FileSystem;"
            "$z=[IO.Compression.ZipFile]::OpenRead($env:ASTRAL_ZIP);"
            "try { $z.Entries | ForEach-Object { $_.FullName } } finally { $z.Dispose() }"};
        tool.extractArgs = base + QStringList{
            "$ErrorActionPreference='Stop'; $ProgressPreference='SilentlyContinue';"
            "Expand-Archive -LiteralPath $env:ASTRAL_ZIP -DestinationPath $env:ASTRAL_DEST -Force"};
    }
#else
    QString unzip = QStandardPaths::findExecutable("unzip");
    if (!unzip.isEmpty()) {
        tool.program = unzip;
        tool.listArgs = {"-Z1", zip};
        tool.extractArgs = {"-q", "-o", zip, "-d", dest};
        return tool;
    }

    QString bsdtar = QStandardPaths::findExecutable("bsdtar");
    if (!bsdtar.isEmpty()) {
        tool.program = bsdtar;
        tool.listArgs = {"-tf", zip};
        tool.extractArgs = {"-xf", zip, "-C", dest};
    }
#endif
    return tool;
}

bool InstallManager::isUnsafeEntry(const QString &entry)
{
    QString path = entry.trimmed();
    if (path.isEmpty())
        return false;
    path.replace('\\', '/');

    if (path.startsWith('/'))
        return true;
    if (path.size() >= 2 && path.at(1) == ':') // C:/ or C:relative
        return true;

    QStringList segments = path.split('/');
    return std::any_of(segments.cbegin(), segments.cend(), [](const QString &s) { return s == ".."; });
}

bool InstallManager::staysInside(const QString &root, QString *offender)
{
    QString base = QFileInfo(root).canonicalFilePath() + '/';

    QDirIterator it(root, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        // symlinks are ok (linux has libfoo.so -> libfoo.so.1) but only inside tje game
        QString resolved = info.isSymLink() ? QDir::cleanPath(info.symLinkTarget()) : info.canonicalFilePath();
        if (!resolved.startsWith(base, PathCase)) {
            *offender = info.filePath();
            return false;
        }
    }
    return true;
}

QString InstallManager::findContentRoot(const QString &extracted)
{
    QDir dir(extracted);
    if (QFileInfo(dir.filePath(GamePart)).isDir())
        return dir.path();

    // explorer zip makes one extra folder around it (MyGame/game/...), mac adds __MACOSX
    QStringList folders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    folders.removeAll("__MACOSX");
    if (folders.size() == 1) {
        QDir inner(dir.filePath(folders.first()));
        if (QFileInfo(inner.filePath(GamePart)).isDir())
            return inner.path();
    }
    return {};
}

bool InstallManager::moveDir(const QString &from, const QString &to)
{
    if (QDir().rename(from, to))
        return true;
    // other drive, then copy
    if (!copyRecursively(from, to)) {
        removePath(to);
        return false;
    }
    QDir(from).removeRecursively();
    return true;
}

bool InstallManager::mergeMissing(const QString &from, const QString &to, QString *error)
{
    if (!QDir().mkpath(to)) {
        *error = QDir::toNativeSeparators(to);
        return false;
    }

    auto entries = QDir(from).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        QString target = QDir(to).filePath(entry.fileName());
        if (entry.isDir() && !entry.isSymLink()) {
            if (!mergeMissing(entry.filePath(), target, error))
                return false;
            continue;
        }
        if (QFileInfo::exists(target) || QFileInfo(target).isSymLink())
            continue; // players file wins
        if (!QFile::rename(entry.filePath(), target) && !QFile::copy(entry.filePath(), target)) {
            *error = QDir::toNativeSeparators(target);
            return false;
        }
    }
    return true;
}
