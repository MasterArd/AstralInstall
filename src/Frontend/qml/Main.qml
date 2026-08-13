import QtQuick
import QtQuick.Window

Window {
    width: 800
    height: 600
    visible: true
    title: "Astral"
    
    Rectangle {
        anchors.fill: parent
        color: "#1e1e2e"
        
        Text {
            anchors.centerIn: parent
            text: "Hello Astral!"
            color: "white"
            font.pixelSize: 24
        }
    }
}