pragma Singleton
import QtQuick
import AstralInstall

QtObject {
    property color color1: Settings.primaryColor
    property color color2: Qt.darker(color1, 1.2)
    property color color3: Qt.darker(color1, 1.4)
}
