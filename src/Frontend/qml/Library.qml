import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

Rectangle {
    color: Colors.bgColor

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // SIDEBAR
        Rectangle {



            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: Colors.panelColor

            // Hairline against the grid
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: Colors.lineColor
            }

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 14

                /*
                    GENRES
                */
                Column {
                    id: genres
                    width: parent.width
                    spacing: 8

                    property bool genresOpen: true

                    // HEADER
                    Rectangle {
                        width: parent.width
                        height: 26
                        color: "transparent"

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Genres"
                            color: genresArea.containsMouse ? Colors.accentColor : Colors.textColor
                            font.pixelSize: 13
                            font.bold: true

                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        Image {
                            id: genresArrow
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            source: genres.genresOpen
                                    ? "qrc:/assets/symboles/dropdown_up.png"
                                    : "qrc:/assets/symboles/dropdown_down.png"
                            sourceSize.height: 16
                            fillMode: Image.PreserveAspectFit
                            visible: false
                        }

                        MultiEffect {
                            source: genresArrow
                            anchors.fill: genresArrow
                            colorization: 1.0
                            colorizationColor: genresArea.containsMouse ? Colors.accentColor : Colors.mutedColor
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: Colors.lineColor
                        }

                        MouseArea {
                            id: genresArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: genres.genresOpen = !genres.genresOpen
                        }
                    }

                    // CONTENT
                    Column {
                        width: parent.width
                        spacing: 6
                        visible: genres.genresOpen

                        FilterCheckBox { text: "Horror" }
                        FilterCheckBox { text: "Simulation" }
                        FilterCheckBox { text: "Action" }
                    }
                }
                /*
                    PLATFORM
                */
                Column {
                    id: platforms
                    width: parent.width
                    spacing: 8

                    property bool platformsOpen: true

                    // HEADER
                    Rectangle {
                        width: parent.width
                        height: 26
                        color: "transparent"

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "platforms"
                            color: platformsArea.containsMouse ? Colors.accentColor : Colors.textColor
                            font.pixelSize: 13
                            font.bold: true

                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        Image {
                            id: platformsArrow
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            source: platforms.platformsOpen
                                    ? "qrc:/assets/symboles/dropdown_up.png"
                                    : "qrc:/assets/symboles/dropdown_down.png"
                            sourceSize.height: 16
                            fillMode: Image.PreserveAspectFit
                            visible: false
                        }

                        MultiEffect {
                            source: platformsArrow
                            anchors.fill: platformsArrow
                            colorization: 1.0
                            colorizationColor: platformsArea.containsMouse ? Colors.accentColor : Colors.mutedColor
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: Colors.lineColor
                        }

                        MouseArea {
                            id: platformsArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: platforms.platformsOpen = !platforms.platformsOpen
                        }
                    }

                    // CONTENT
                    Column {
                        width: parent.width
                        spacing: 6
                        visible: platforms.platformsOpen

                        FilterCheckBox { text: "Windows" }
                        FilterCheckBox { text: "Linux" }
                    }
                }
            }
        }
        //main part
        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: gameLibrary

            property int columns: 3
            cellWidth: width / columns
            cellHeight: cellWidth

            delegate: GameCard {
                required property var model

                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                name: model.name
                description: model.description
                developer: model.developer
                banner: model.banner
                platformWindows: model.platformWindows
                platformLinux: model.platformLinux
                genres: model.genres
                version: model.version
            }
            Text {
                anchors.centerIn: parent
                visible: gameLibrary.count === 0
                text: "No games"
                color: Colors.mutedColor
                font.pixelSize: 13
            }
        }
    }
}
