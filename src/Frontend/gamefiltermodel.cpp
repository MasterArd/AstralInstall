#include "gamefiltermodel.h"

#include <QAbstractItemModel>
#include <QModelIndex>

GameFilterModel::GameFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Re-run filterAcceptsRow() whenever the source model changes and
    // whenever invalidateFilter() is called from a setter below.
    setDynamicSortFilter(true);
}

void GameFilterModel::setHorror(bool value)
{
    if (m_horror == value)
        return;
    m_horror = value;
    invalidateFilter();
    emit filtersChanged();
}

void GameFilterModel::setSimulation(bool value)
{
    if (m_simulation == value)
        return;
    m_simulation = value;
    invalidateFilter();
    emit filtersChanged();
}

void GameFilterModel::setAction(bool value)
{
    if (m_action == value)
        return;
    m_action = value;
    invalidateFilter();
    emit filtersChanged();
}

void GameFilterModel::setWindows(bool value)
{
    if (m_windows == value)
        return;
    m_windows = value;
    invalidateFilter();
    emit filtersChanged();
}

void GameFilterModel::setLinux(bool value)
{
    if (m_linux == value)
        return;
    m_linux = value;
    invalidateFilter();
    emit filtersChanged();
}

bool GameFilterModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex &sourceParent) const
{
    const QAbstractItemModel *model = sourceModel();
    if (!model)
        return true;

    const QModelIndex index = model->index(sourceRow, 0, sourceParent);
    if (!index.isValid())
        return true;

    const QHash<int, QByteArray> roles = model->roleNames();
    const int genresRole          = roles.key("genres");
    const int platformWindowsRole = roles.key("platformWindows");
    const int platformLinuxRole   = roles.key("platformLinux");

    const QStringList genres = model->data(index, genresRole).toStringList();
    const bool hasWindows    = model->data(index, platformWindowsRole).toBool();
    const bool hasLinux      = model->data(index, platformLinuxRole).toBool();

    //filter
    const bool anyGenreChecked = m_horror || m_simulation || m_action;
    const bool genreMatch =
    (m_horror     && genres.contains(QStringLiteral("horror"),     Qt::CaseInsensitive))
    || (m_simulation && genres.contains(QStringLiteral("simulation"), Qt::CaseInsensitive))
    || (m_action     && genres.contains(QStringLiteral("action"),     Qt::CaseInsensitive));

    //platform
    const bool anyPlatformChecked = m_windows || m_linux;
    const bool platformMatch =
    (m_windows && hasWindows)
    || (m_linux   && hasLinux);

    return genreMatch && platformMatch;
}