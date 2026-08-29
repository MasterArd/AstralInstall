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
            id: infoArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height * 2 / 3
            color: "black"

            //name + version
            RowLayout {
                id: titleRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8

                //game name
                Rectangle {
                    // Box waechst mit dem Text mit -> kein Leerraum links
                    width: nameText.implicitWidth
                    height: 25
                    color: "transparent"
                    //border.color: "white"
                    //border.width: 1

                    Text {
                        id: nameText
                        anchors.centerIn: parent
                        color: "white"
                        text: card.name
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Item { Layout.fillWidth: true }//placeholder
                //game version
                Rectangle {
                    width: 50
                    height: 25
                    color: "black"
                    border.color: "white"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 8
                        color: "white"
                        text: "v" + card.version
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }//name + version finish

            //dev name
            Row {
                anchors.left: parent.left
                anchors.top: titleRow.bottom
                anchors.margins: 8
                spacing: 0

                Text {
                    id: byLabel
                    color: "white"
                    text: "by "
                    font.pixelSize: 12
                }

                Text {
                    width: Math.min(implicitWidth, infoArea.width - 16 - byLabel.width)
                    color: hoverHandler.hovered ? "green" : "white"
                    text: "@" + card.developer
                    font.pixelSize: 12
                    elide: Text.ElideRight

                    HoverHandler {
                        id: hoverHandler
                    }
                }
            }

        }
    }
}
