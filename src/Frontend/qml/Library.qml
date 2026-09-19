import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

Rectangle {
    id: root
    color: Colors.bgColor

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // SIDEBAR
        Rectangle {



            Layout.preferredWidth: Ui.panelWidth
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
                anchors.margins: Ui.margin
                spacing: Ui.margin

                /*
                    GENRES
                */
                Column {
                    id: genres
                    width: parent.width
                    spacing: Ui.spacing

                    property bool genresOpen: true

                    // HEADER
                    Rectangle {
                        width: parent.width
                        height: Ui.headerHeight
                        color: "transparent"

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Genres"
                            color: genresArea.containsMouse ? Colors.accentColor : Colors.textColor
                            font.pixelSize: Ui.fontSize
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
                            sourceSize.height: Ui.arrowSize
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
                        spacing: Ui.itemSpacing
                        visible: genres.genresOpen

                        FilterCheckBox { 
                            text: "horror"
                            checked: gameFilter.horror
                            onToggled: gameFilter.horror = checked 
                        }
                        FilterCheckBox { 
                            text: "simulation" 
                            checked: gameFilter.simulation
                            onToggled: gameFilter.simulation = checked
                        }
                        FilterCheckBox { 
                            text: "action" 
                            checked: gameFilter.action
                            onToggled: gameFilter.action = checked
                        }
                    }
                }
                /*
                    PLATFORM
                */
                Column {
                    id: platforms
                    width: parent.width
                    spacing: Ui.spacing

                    property bool platformsOpen: true

                    // HEADER
                    Rectangle {
                        width: parent.width
                        height: Ui.headerHeight
                        color: "transparent"

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "platforms"
                            color: platformsArea.containsMouse ? Colors.accentColor : Colors.textColor
                            font.pixelSize: Ui.fontSize
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
                            sourceSize.height: Ui.arrowSize
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
                        spacing: Ui.itemSpacing
                        visible: platforms.platformsOpen

                        FilterCheckBox { 
                            text: "windows" 
                            checked: gameFilter.windows
                            onToggled: gameFilter.windows = checked
                        }
                        FilterCheckBox { 
                            text: "linux" 
                            checked: gameFilter.linux
                            onToggled: gameFilter.linux = checked
                        }
                    }
                }
            }
        }
        //main part
        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: gameFilter
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
                text: "No games"
                visible: gameFilter.rowCount() === 0
                color: Colors.mutedColor
                font.pixelSize: 13
            }
        }
    }
}
