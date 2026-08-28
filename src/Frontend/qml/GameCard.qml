import QtQuick

/*
 * One tile in the library grid. Library.qml fills the properties below
 * from the model, so the data side stays put no matter how this is
 * styled. Everything past that is placeholder - restyle freely.
 */
Item {
    id: card

    property string name: ""
    property string description: ""
    property string developer: ""
    property string capsule: ""
    property string banner: ""
    property var platforms: []
    property var genres: []
    property string version: ""

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 0
        color: "transparent"

        // Top third
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height / 3
            color: "red"
        }

        // Bottom two thirds
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height * 2 / 3
            color: "blue"
        }
    }
}
