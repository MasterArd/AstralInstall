import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects


Window {
    width: 800
    height: 600
    visible: true
    title: "Astral"
    color: Colors.bgColor

    // Top bar
    Rectangle {
        id: navBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: 44
        color: Colors.panelColor

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Colors.lineColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10

            Button {
                id: libraryButton
                text: "library"

                Layout.preferredWidth: 100
                Layout.fillHeight: true

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true

                background: Rectangle {
                    color: "transparent"

                    // Active page marker
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        color: Colors.accentColor
                        visible: gameManager.currentPage === 0
                    }
                }
                contentItem: Text {
                    text: libraryButton.text
                    color: gameManager.currentPage === 0
                           ? Colors.textColor
                           : libraryButton.hovered ? Colors.accentColor : Colors.mutedColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 15
                    font.bold: gameManager.currentPage === 0

                    Behavior on color { ColorAnimation { duration: 120 } }
                }

                onClicked: gameManager.currentPage = 0
            }
            Button {
                id: myGamesButton
                text: "My games"

                Layout.preferredWidth: 100
                Layout.fillHeight: true

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true

                background: Rectangle {
                    color: "transparent"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        color: Colors.accentColor
                        visible: gameManager.currentPage === 1
                    }
                }
                contentItem: Text {
                    text: myGamesButton.text
                    color: gameManager.currentPage === 1
                           ? Colors.textColor
                           : myGamesButton.hovered ? Colors.accentColor : Colors.mutedColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 15
                    font.bold: gameManager.currentPage === 1

                    Behavior on color { ColorAnimation { duration: 120 } }
                }

                onClicked: gameManager.currentPage = 1
            }
            //gh login
            Button {
                id: githubButton
                text: "GitHub"

                Layout.preferredWidth: 100
                Layout.preferredHeight: 26
                Layout.alignment: Qt.AlignVCenter

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true

                background: Rectangle {
                    color: Colors.bgColor
                    border.color: githubButton.hovered ? Colors.accentColor : Colors.lineColor
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
                contentItem: Row {
                    spacing: 6
                    leftPadding: 10
                    Image {
                        source: "qrc:/assets/symboles/Githublogo.png"
                        sourceSize.width: 14
                        sourceSize.height: 14
                        width: 14
                        height: 14
                        fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "GitHub"
                        color: Colors.accentColor
                        font.pixelSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                onClicked: {}//make githb login
            }

            Item {
                Layout.fillWidth: true
            }

            TextField {
                id: searchField
                Layout.preferredWidth: 200
                Layout.preferredHeight: 26
                Layout.alignment: Qt.AlignVCenter
                color: Colors.textColor
                placeholderText: "Search..."
                placeholderTextColor: Colors.mutedColor
                font.pixelSize: 12
                leftPadding: 10
                rightPadding: 10

                background: Rectangle {
                    color: Colors.bgColor
                    border.color: searchField.activeFocus ? Colors.accentColor : Colors.lineColor
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
            }
            Item {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter

                Image {
                    id: dotsMask
                    anchors.fill: parent
                    source: "qrc:/assets/symboles/dots.png"
                    sourceSize.height: 20
                    fillMode: Image.PreserveAspectFit
                    visible: false
                }
                Rectangle {
                    anchors.fill: parent
                    color: dotsArea.containsMouse ? Colors.accentColor : Colors.mutedColor
                    layer.enabled: true
                    layer.effect: MultiEffect {
                        maskEnabled: true
                        maskSource: dotsMask
                    }

                    Behavior on color { ColorAnimation { duration: 120 } }
                }
                MouseArea {
                    id: dotsArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: testConsole.show()
                }
            }
        }
    }

    StackLayout {
        anchors.top: navBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: gameManager.currentPage

        Library { }
        MyGamesPage { }
    }
}
