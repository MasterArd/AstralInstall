import QtQuick
import QtQuick.Layouts
import QtQuick.Controls


Rectangle {
    color: Colors.bgColor

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
