#ifndef BACKENDBRIDGE_H
#define BACKENDBRIDGE_H

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QByteArray>
#include <QTimer>

class QJsonObject;

/*
 * Runs the Go backend as a child process and talks to it over
 * stdin/stdout: one JSON object per line, see README.
 *
 * The backend handles requests strictly one at a time and does not
 * echo a request id. So we keep the open requests in a FIFO queue and
 * match responses to them in order.
 */
class BackendBridge : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    explicit BackendBridge(QObject *parent = nullptr);
    ~BackendBridge() override;

    bool isRunning() const;

public slots:
    // Starts the child process. Returns false if the binary is missing
    // or fails to start.
    bool start();

    // Closes stdin so the backend's read loop ends, then waits briefly
    // for it to exit on its own.
    void stop();

    // Sends action "check_release".
    // Answers with releaseChecked() or requestFailed().
    void checkRelease(const QString &repo);

    // Sends action "download_newest": downloads the newest release
    // asset into destination. platform is a filter on the asset name
    // ("windows", "linux", "darwin", ...); empty takes the first asset.
    // An empty destination lets the backend store it in its own working
    // directory.
    // Answers with downloadFinished() or requestFailed().
    void downloadNewest(const QString &repo,
                        const QString &platform = QString(),
                        const QString &destination = QString());

signals:
    void runningChanged();

    // Successful response to checkRelease()
    void releaseChecked(const QString &repo, const QString &version);

    // Successful response to downloadNewest()
    void downloadFinished(const QString &repo, const QString &destination);

    // The backend rejected the request: unknown action, network error,
    // repo not found, timeout, ...
    void requestFailed(const QString &repo, const QString &error);

    // Trouble with the process itself, not with a single request
    void backendError(const QString &message);

    // Anything the backend prints that is not a response: stderr logs
    // and progress lines on stdout. Purely informational.
    void backendMessage(const QString &text);

private slots:
    void onStdout();
    void onStderr();
    void onProcessError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus status);
    void onTimeout();

private:
    struct PendingRequest {
        QString action;
        QString repo;
        QString destination;
        bool timedOut = false;
    };

    void sendRequest(const QJsonObject &request, const PendingRequest &pending);
    void handleLine(const QByteArray &line);
    void failAllPending(const QString &reason);
    void armTimeout();

    static QString backendExecutablePath();
    static int timeoutForAction(const QString &action);

    QProcess m_process;
    QByteArray m_buffer;
    QQueue<PendingRequest> m_pending;
    QTimer m_timeoutTimer;
};

#endif // BACKENDBRIDGE_H
