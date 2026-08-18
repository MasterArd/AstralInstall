#include "settings.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>

namespace {

const QColor DefaultPrimaryColor = QColor(QStringLiteral("#1a4d3d"));

constexpr int SchemaVersion = 1;

} 

Settings::Settings(QObject *parent)
    : QObject(parent), m_primaryColor(DefaultPrimaryColor)
{
    load();
}

QString Settings::filePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(dir).filePath(QStringLiteral("settings.json"));
}

void Settings::setPrimaryColor(const QColor &color)
{
    if (!color.isValid() || color == m_primaryColor)
        return;

    m_primaryColor = color;
    emit primaryColorChanged();
}

void Settings::load()
{
    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly))
        return; // No config yet - keep the defaults.

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("[Settings] %s is not valid JSON: %s",
                 qUtf8Printable(filePath()), qUtf8Printable(parseError.errorString()));
        return;
    }

    const QJsonObject root = doc.object();
    const int version = root.value(QStringLiteral("version")).toInt(SchemaVersion);
    if (version > SchemaVersion) {
        qWarning("[Settings] config version %d is newer than supported %d, ignoring it",
                 version, SchemaVersion);
        return;
    }

    const QJsonObject theme = root.value(QStringLiteral("theme")).toObject();
    const QString primaryText = theme.value(QStringLiteral("primaryColor")).toString();
    if (!primaryText.isEmpty()) {
        const QColor primary(primaryText);
        if (primary.isValid())
            m_primaryColor = primary;
        else
            qWarning("[Settings] theme.primaryColor \"%s\" is not a color, keeping %s. "
                     "Expected \"#RRGGBB\" or an SVG color name.",
                     qUtf8Printable(primaryText), qUtf8Printable(m_primaryColor.name()));
    }
}
