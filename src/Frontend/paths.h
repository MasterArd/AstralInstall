#ifndef PATHS_H
#define PATHS_H

#include <QString>

// all paths where we write stuff. NEVER next to the exe (program files is read only!)
// the getters create the folder if its not there
class Paths
{
public:
    static QString dataDir();   // %LOCALAPPDATA%\AstralInstall
    static QString gamesRoot(); // dataDir/games or from settings.json
    static void setGamesRoot(const QString &dir);
    static QString stagingDir(); // inside gamesRoot so its the same drive (rename)
    static QString logDir();
    static QString configFile();
    static QString lockFile();
};

#endif
