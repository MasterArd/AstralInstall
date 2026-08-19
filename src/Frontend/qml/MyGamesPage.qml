import QtQuick
import QtQuick.Layouts
import QtQuick.Controls


Rectangle {
    color: Colors.color1
    
    Rectangle {
        width: parent.width
        height: parent.height
        
        // CONTENT (oben)
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 60
            color: Colors.color1

            
        }
        
        // FOOTER (unten)
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: Colors.color3

            
            ProgressBar {
                width: parent.width - 20
                height: 10
                value: 0.45  // 45%
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                background: Rectangle {
                    color: "#333"
                    radius: 5
                }
                
                contentItem: Rectangle {
                    width: parent.width * parent.value
                    color: Colors.color1
                    radius: 5
                }
            }
            
        }
    }
}