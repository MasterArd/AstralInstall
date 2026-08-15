import QtQuick
import QtQuick.Layouts

Rectangle {
    color: Colors.maincolor

    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // SIDEBAR
        Rectangle {
            


            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: Colors.color2

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                
                Text {
                    text: "test" 
                    color: "white"
                    font.pixelSize: 40
                }
            }
        }
        
        // MAIN CONTENT (GridView)
        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
}