#ifndef GAMEFILTERMODEL_H
#define GAMEFILTERMODEL_H

#include <QSortFilterProxyModel>

// all the filter stuff
class GameFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool horror READ horror WRITE setHorror NOTIFY filtersChanged)
    Q_PROPERTY(bool simulation READ simulation WRITE setSimulation NOTIFY filtersChanged)
    Q_PROPERTY(bool action READ action WRITE setAction NOTIFY filtersChanged)
    Q_PROPERTY(bool windows READ windows WRITE setWindows NOTIFY filtersChanged)
    Q_PROPERTY(bool isLinux READ isLinux WRITE setIsLinux NOTIFY filtersChanged)

public:
    explicit GameFilterModel(QObject *parent = nullptr);

    bool horror() const { return m_horror; }
    bool simulation() const { return m_simulation; }
    bool action() const { return m_action; }
    bool windows() const { return m_windows; }
    bool isLinux() const { return m_isLinux; }

public slots:
    void setHorror(bool value);
    void setSimulation(bool value);
    void setAction(bool value);
    void setWindows(bool value);
    void setIsLinux(bool value);

signals:
    void filtersChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    // Defaults match the QML checkboxes: everything starts checked.
    bool m_horror = true;
    bool m_simulation = true;
    bool m_action = true;
    bool m_windows = true;
    bool m_isLinux = true;
};

#endif