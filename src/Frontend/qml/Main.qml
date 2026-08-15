import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 800
    height: 600
    visible: true
    title: "Astral"
    color: Colors.color3
    
    Row {
        id: navBar
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