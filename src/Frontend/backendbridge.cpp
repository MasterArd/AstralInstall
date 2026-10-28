#include "backendbridge.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace {
// GitHub can hang and the backend has no timeout of its own, so
// without this the UI would wait forever.
constexpr int kRequestTimeoutMs = 15000;
// A download fetches a whole release file, which legitimately takes
// longer than an API query.
constexpr int kDownloadTimeoutMs = 10 * 60 * 1000;
constexpr int kLoginTimeoutMs = 30 * 60 * 1000;
constexpr int kProcessWaitMs = 3000;

const QString kActionCheckRelease = QStringLiteral("check_release");
const QString kActionDownloadNewest = QStringLiteral("download_newest");
const QString kActionLogin = QStringLiteral("login");
}

BackendBridge::BackendBridge(QObject *parent)
    : QObject(parent)
{
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &BackendBridge::onTimeout);

    connect(&m_process, &QProcess::readyReadStandardOutput,
            this, &BackendBridge::onStdout);
    connect(&m_process, &QProcess::readyReadStandardError,
            this, &BackendBridge::onStderr);
    connect(&m_process, &QProcess::errorOccurred,
            this, &BackendBridge::onProcessError);
    connect(&m_process, &QProcess::finished,
            this, &BackendBridge::onFinished);
}

BackendBridge::~BackendBridge()
{
    stop();
}

bool BackendBridge::isRunning() const
{
    return m_process.state() == QProcess::Running;
}

QString BackendBridge::backendExecutablePath()
{
    // "go build ." names the binary after the module: Backend on Linux,
    // Backend.exe on Windows. CMake copies it next to the frontend.
#ifdef Q_OS_WIN
    const QString name = QStringLiteral("Backend.exe");
#else
    const QString name = QStringLiteral("Backend");
#endif
    return QDir(QCoreApplication::applicationDirPath()).filePath(name);
}

bool BackendBridge::start()
{
    if (isRunning())
        return true;

    const QString path = backendExecutablePath();
    if (!QFileInfo::exists(path)) {
        emit backendError(tr("Backend not found: %1").arg(path));
        return false;
    }

    // No arguments puts the backend straight into IPC mode.
    // (--debug would run its self tests instead.)
    m_process.start(path, QStringList{});

    if (!m_process.waitForStarted(kProcessWaitMs)) {
        emit backendError(tr("Failed to start backend: %1")
                              .arg(m_process.errorString()));
        return false;
    }

    emit runningChanged();
    return true;
}

void BackendBridge::stop()
{
    if (m_process.state() == QProcess::NotRunning)
        return;

    m_timeoutTimer.stop();

    // Closing stdin is enough: the scanner in the backend runs out and
    // the process exits by itself.
    m_process.closeWriteChannel();

    if (!m_process.waitForFinished(kProcessWaitMs)) {
        m_process.kill();
        m_process.waitForFinished(kProcessWaitMs);
    }
}

void BackendBridge::checkRelease(const QString &repo)
{
    if (repo.trimmed().isEmpty()) {
        emit requestFailed(repo, tr("No repository given"));
        return;
    }

    QJsonObject request;
    request[QStringLiteral("action")] = kActionCheckRelease;
    request[QStringLiteral("repo")] = repo;

    sendRequest(request, PendingRequest{kActionCheckRelease, repo, QString(), false});
}

void BackendBridge::downloadNewest(const QString &repo,
                                   const QString &platform,
                                   const QString &destination)
{
    if (repo.trimmed().isEmpty()) {
        emit requestFailed(repo, tr("No repository given"));
        return;
    }

    QJsonObject request;
    request[QStringLiteral("action")] = kActionDownloadNewest;
    request[QStringLiteral("repo")] = repo;
    request[QStringLiteral("platform")] = platform;
    request[QStringLiteral("destination")] = destination;

    sendRequest(request,
                PendingRequest{kActionDownloadNewest, repo, destination, false});
}

void BackendBridge::login()
{
    for (const PendingRequest &pending : m_pending) {
        if (pending.action == kActionLogin)
            return;
    }

    m_lastLoginError.clear();
    QJsonObject request;
    request[QStringLiteral("action")] = kActionLogin;
    sendRequest(request, PendingRequest{kActionLogin, QString(), QString(), false});
}

void BackendBridge::sendRequest(const QJsonObject &request, const PendingRequest &pending)
{
    if (!isRunning() && !start()) {
        if (pending.action == kActionLogin)
            emit loginFailed(tr("Backend is not running"));
        else
            emit requestFailed(pending.repo, tr("Backend is not running"));
        return;
    }

    // Compact so the message stays on a single line - the backend reads
    // line by line.
    QByteArray line = QJsonDocument(request).toJson(QJsonDocument::Compact);
    line.append('\n');

    if (m_process.write(line) == -1) {
        const QString error = tr("Failed to write to backend: %1")
                                  .arg(m_process.errorString());
        if (pending.action == kActionLogin)
            emit loginFailed(error);
        else
            emit requestFailed(pending.repo, error);
        return;
    }

    m_pending.enqueue(pending);
    armTimeout();
}

void BackendBridge::onStdout()
{
    m_buffer.append(m_process.readAllStandardOutput());

    // Several responses can arrive in one read, and a single response
    // can be split across two. So buffer and only cut a line once we
    // actually see a newline.
    int newline;
    while ((newline = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(newline);
        m_buffer.remove(0, newline + 1);

        if (line.endsWith('\r'))
            line.chop(1);

        if (!line.trimmed().isEmpty())
            handleLine(line);
    }
}

void BackendBridge::handleLine(const QByteArray &line)
{
    if (!m_pending.isEmpty() && m_pending.head().action == kActionLogin) {
        const QString text = QString::fromUtf8(line);
        const QString successPrefix = QStringLiteral("Logged in as ");
        const int idSeparator = text.indexOf(QStringLiteral(" id "),
                                             successPrefix.size());
        if (text.startsWith(successPrefix) && idSeparator > successPrefix.size()) {
            const QString login = text.mid(successPrefix.size(),
                                           idSeparator - successPrefix.size());
            const PendingRequest pending = m_pending.dequeue();
            m_timeoutTimer.stop();
            armTimeout();
            if (!pending.timedOut)
                emit loginSucceeded(login);
            return;
        }
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        // While downloading, the backend writes progress to stdout
        // ("Latest release: ...", "Downloading: ..."), so into the same
        // channel as the responses. Such lines are not a protocol
        // error - the actual JSON response still follows. So pass them
        // through and keep waiting instead of clearing the open
        // request.
        // The current backend prints the access token on its success line.
        // Do not forward login output to UI logs or other listeners.
        if (m_pending.isEmpty() || m_pending.head().action != kActionLogin)
            emit backendMessage(QString::fromUtf8(line));
        return;
    }

    if (m_pending.isEmpty()) {
        // Should not happen while the backend only ever answers
        // requests. Once it starts pushing events on its own (download
        // progress, for example), handle them here.
        emit backendError(tr("Unexpected response from backend: %1")
                              .arg(QString::fromUtf8(line)));
        return;
    }

    const PendingRequest &head = m_pending.head();
    const QJsonObject obj = doc.object();
    if (head.action == kActionLogin) {
        const QString url = obj.value(QStringLiteral("URL")).toString();
        const QString userCode = obj.value(QStringLiteral("UserCode")).toString();
        if (obj.value(QStringLiteral("success")).toBool()
            && !url.isEmpty() && !userCode.isEmpty()) {
            m_timeoutTimer.stop();
            armTimeout();
            emit loginDeviceCodeReceived(url, userCode);
            return;
        }

        const PendingRequest pending = m_pending.dequeue();
        m_timeoutTimer.stop();
        armTimeout();
        if (!pending.timedOut) {
            const QString error = obj.value(QStringLiteral("error")).toString(
                tr("Backend returned an incomplete GitHub login response"));
            emit loginFailed(error);
        }
        return;
    }

    const PendingRequest pending = m_pending.dequeue();
    m_timeoutTimer.stop();
    armTimeout();

    // Nobody is waiting on a request that already timed out, but we
    // still had to pop it so the queue stays aligned.
    if (pending.timedOut)
        return;

    if (!obj.value(QStringLiteral("success")).toBool()) {
        const QString error = obj.value(QStringLiteral("error"))
                                  .toString(tr("Unknown error"));
        if (pending.action == kActionLogin)
            emit loginFailed(error);
        else
            emit requestFailed(pending.repo, error);
        return;
    }

    if (pending.action == kActionCheckRelease) {
        emit releaseChecked(pending.repo,
                            obj.value(QStringLiteral("version")).toString());
    } else if (pending.action == kActionDownloadNewest) {
        emit downloadFinished(pending.repo, pending.destination);
    }
}

void BackendBridge::armTimeout()
{
    if (m_pending.isEmpty()) {
        m_timeoutTimer.stop();
        return;
    }

    if (!m_timeoutTimer.isActive())
        m_timeoutTimer.start(timeoutForAction(m_pending.head().action));
}

int BackendBridge::timeoutForAction(const QString &action)
{
    if (action == kActionDownloadNewest)
        return kDownloadTimeoutMs;
    if (action == kActionLogin)
        return kLoginTimeoutMs;
    return kRequestTimeoutMs;
}

void BackendBridge::onTimeout()
{
    if (m_pending.isEmpty())
        return;

    // Only mark the oldest request, do not drop it: the backend works
    // serially and will still answer eventually. That answer is what
    // clears the entry.
    PendingRequest &head = m_pending.head();
    if (!head.timedOut) {
        head.timedOut = true;
        if (head.action == kActionLogin)
            emit loginFailed(tr("GitHub login timed out"));
        else
            emit requestFailed(head.repo, tr("Request timed out"));
    }

    m_timeoutTimer.start(timeoutForAction(head.action));
}

void BackendBridge::onStderr()
{
    // The backend logs to stderr. Keep it out of the protocol stream,
    // just pass it through.
    const QByteArray err = m_process.readAllStandardError().trimmed();
    if (!err.isEmpty()) {
        if (!m_pending.isEmpty() && m_pending.head().action == kActionLogin)
            m_lastLoginError = QString::fromUtf8(err);
        qWarning("[Backend] %s", err.constData());
        emit backendMessage(QString::fromUtf8(err));
    }
}

void BackendBridge::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    emit backendError(tr("Backend process error: %1").arg(m_process.errorString()));
}

void BackendBridge::onFinished(int exitCode, QProcess::ExitStatus status)
{
    m_timeoutTimer.stop();
    onStdout();
    onStderr();
    m_buffer.clear();

    failAllPending(tr("Backend stopped"));

    if (status == QProcess::CrashExit)
        emit backendError(tr("Backend crashed"));
    else if (exitCode != 0)
        emit backendError(tr("Backend exited with code %1").arg(exitCode));

    emit runningChanged();
}

void BackendBridge::failAllPending(const QString &reason)
{
    while (!m_pending.isEmpty()) {
        const PendingRequest pending = m_pending.dequeue();
        if (pending.timedOut)
            continue;
        if (pending.action == kActionLogin) {
            const QString error = m_lastLoginError.isEmpty()
                                      ? tr("GitHub login failed: %1").arg(reason)
                                      : m_lastLoginError;
            emit loginFailed(error);
        } else {
            emit requestFailed(pending.repo, reason);
        }
    }
    m_lastLoginError.clear();
}
