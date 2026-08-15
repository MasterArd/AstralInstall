#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QObject>
#include <QString>

/*
 * Frontend-only user preferences, exposed to QML as the singleton
 * "Settings" (see main.cpp). Colors.qml derives the whole palette
 * from primaryColor, so writing to it repaints the UI live.
 *
 * Anything the Go backend also needs belongs in the IPC in
 * BackendBridge, not in here.
 *
 * Loads from disk on startup; saving is not implemented yet.
 */
class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QColor primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)

public:
    explicit Settings(QObject *parent = nullptr);

    QColor primaryColor() const { return m_primaryColor; }
    void setPrimaryColor(const QColor &color);

    // Absolute path of the config file, below %LOCALAPPDATA% on Windows.
    // Nothing writes it yet, load() just reads it if it happens to exist.
    static QString filePath();

signals:
    void primaryColorChanged();

private:
    void load();

    QColor m_primaryColor;
};

#endif
