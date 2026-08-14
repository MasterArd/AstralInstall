import QtQuick
import QtQuick.Layouts

Rectangle {
    color: "#f0f0f0"
    
    GridView {
        anchors.fill: parent
        clip: true
        model: 30

        property int columns: Math.max(1, Math.floor(width / 200))
        cellWidth: width / columns
        cellHeight: cellWidth * 1.4

        delegate: Item {
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: 8
                radius: 4
                color: "red"
            }
        }
    }
}