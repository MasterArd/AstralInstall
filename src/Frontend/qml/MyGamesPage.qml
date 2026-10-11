import QtQuick
import QtQuick.Layouts
import QtQuick.Controls


Rectangle {
    id: root
    color: Colors.bgColor

    property var gamesModel: gamesList

    ListModel {
        id: gamesList

        ListElement {
            name: "Test_Name"
            banner: "https://github.com/Hannes-swd/Astrallinstall_Template/raw/main/AstralInstall/Banner.png"
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: Colors.bgColor

        // CONTENT (top)
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 60
            color: Colors.bgColor

            //game cards

            GridView {
                id: gamesGrid
                anchors.fill: parent
                anchors.margins: 16
                clip: true
                model: root.gamesModel

                property int columns: Math.max(1, Math.floor(width / 240))
                cellWidth: width / columns
                cellHeight: 280

                delegate: MyGameCard {
                    required property var model

                    width: gamesGrid.cellWidth
                    height: gamesGrid.cellHeight
                    name: model.name
                    banner: model.banner
                }

                Text {
                    anchors.centerIn: parent
                    text: "Noch keine installierten Spiele"
                    color: Colors.mutedColor
                    font.pixelSize: 14
                    visible: gamesGrid.count === 0
                }
            }

        }

        // FOOTER (bottom)
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: Colors.panelColor

            // Hairline against the content
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: Colors.lineColor
            }

            ProgressBar {
                width: parent.width - 28
                height: 6
                value: 0.45  // 45%
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                background: Rectangle {
                    color: Colors.bgColor
                    border.color: Colors.lineColor
                    border.width: 1
                }

                contentItem: Rectangle {
                    width: parent.width * parent.value
                    color: Colors.accentColor
                }
            }

        }
    }
}
