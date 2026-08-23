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
    color: Colors.color3

    RowLayout {
        id: navBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        spacing: 10

        Button {
            text: "library" 
            width: 100
            height: 40
            
            hoverEnabled: false
            focusPolicy: Qt.NoFocus
            flat: true

            background: Rectangle {
                color: "transparent" 
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 20
            }

            onClicked: gameManager.currentPage = 0
        }
        Button { 
            text: "My games" 
            
            width: 100
            height: 40
            
            hoverEnabled: false
            focusPolicy: Qt.NoFocus
            flat: true
            
            background: Rectangle {
                color: "transparent" 
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 20
            }

            onClicked: gameManager.currentPage = 1
        }
        //gh login
        Button { 
            text: "GitHub" 
            
            width: 100
            height: 40
            
            hoverEnabled: false
            focusPolicy: Qt.NoFocus
            flat: true
            
            background: Rectangle {
                color: "Black" 
                radius: 20
            }
            contentItem: Row {
                spacing: 6
                leftPadding: 10
                Image {
                    source: "qrc:/assets/symboles/Githublogo.png"
                    sourceSize.width: 20
                    sourceSize.height: 20
                    width: 20
                    height: 20
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "GitHub"
                    color: "white"
                    font.pixelSize: 18
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
            Layout.preferredHeight: 40
            color: Colors.color3
            placeholderText: "Search..."

            background: Rectangle {
                radius: 5 
                color: "white"
                border.color: "#005a05"
                border.width: 1
            }
        }
        Item {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 10

            Image {
                id: dotsMask
                anchors.fill: parent
                source: "qrc:/assets/symboles/dots.png"
                sourceSize.height: 24
                fillMode: Image.PreserveAspectFit
                visible: false
            }
            Rectangle {
                anchors.fill: parent
                color: "white"
                layer.enabled: true
                layer.effect: MultiEffect {
                    maskEnabled: true
                    maskSource: dotsMask
                }
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: testConsole.show()
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