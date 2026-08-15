pragma Singleton
import QtQuick
import AstralInstall

QtObject {
    property color maincolor: Settings.primaryColor
    property color color2: Qt.darker(maincolor, 1.2)
    property color color3: Qt.darker(maincolor, 1.4)
}
