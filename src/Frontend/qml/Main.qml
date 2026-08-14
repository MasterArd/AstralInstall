import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    width: 800
    height: 600
    visible: true
    title: "Astral"
    
    Row {
        Button { 
            text: "Shop" 
            width: 100
            height: 40
            font.pixelSize: 16
            hoverEnabled: false

            onClicked: gamemanager.currentPage = 1
            }
        Button { 
            text: "My games" 
            width: 150
            height: 40
            font.pixelSize: 16
            hoverEnabled: false

            onClicked: gamemanager.currentPage = 2
            }
    }
}