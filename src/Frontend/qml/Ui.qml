pragma Singleton
import QtQuick

QtObject {
    // font size stuff 
    property real scale: 1.3

    readonly property int fontSize:    Math.round(13 * scale)
    readonly property int headerHeight: Math.round(26 * scale)
    readonly property int boxSize:     Math.round(16 * scale)
    readonly property int arrowSize:   Math.round(16 * scale)
    readonly property int panelWidth:  Math.round(200 * scale)
    readonly property int margin:      Math.round(14 * scale)
    readonly property int spacing:     Math.round(8 * scale)
    readonly property int itemSpacing: Math.round(6 * scale)
}
