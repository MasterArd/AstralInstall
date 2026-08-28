import QtQuick
import QtQuick.Layouts

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
            Image {
                height: parent.height
                width: parent.width
                source: card.banner
            }
        }

        // Bottom two thirds
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height * 2 / 3
            color: "black"

            //name + version
            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
                Text { color: "white"; text: card.name;  font.pixelSize: 14 }
                Item { Layout.fillWidth: true }//placeholder
                Text { color: "white"; text: "v" + card.version;  font.pixelSize: 14 }
            }
        }
    }
}
