#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>

class GameManager : public QObject {
    Q_OBJECT

public:
    enum Page { Shop = 0, MyGames = 1};
    Q_ENUM(Page)

private:
    Q_PROPERTY(Page currentPage READ getCurrentPage WRITE setCurrentPage NOTIFY pageChanged)

public:
    explicit GameManager(QObject *parent = nullptr);

    Page getCurrentPage() const { return m_currentPage; }
    void setCurrentPage(Page page) {
        if (m_currentPage != page) {
            m_currentPage = page;
            emit pageChanged();
        }
    }
    
signals:
    void pageChanged();
    
private:
    Page m_currentPage = Shop;
};

#endif