import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 800
    height: 600
    visible: true
    title: "Astral"
    
    Row {
        id: navBar
        Button {
            text: "library" 
            width: 100
            height: 40
            font.pixelSize: 16
            hoverEnabled: false

            onClicked: gameManager.currentPage = 0
            }
        Button { 
            text: "My games" 
            width: 150
            height: 40
            font.pixelSize: 16
            hoverEnabled: false

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