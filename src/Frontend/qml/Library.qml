import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

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
                
                /*
                    GENRES
                */
                Column {
                    id: genres
                    width: parent.width
                    spacing: 10
                    
                    property bool genresOpen: true
                    
                    // HEADER
                    Rectangle {
                        width: parent.width
                        height: 40
                        color: "transparent"
                        
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Genres"
                            color: "white"
                            font.pointSize: 20
                        }
                        
                        Image {
                            id: genresArrow
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            source: genres.genresOpen
                                    ? "qrc:/assets/symboles/dropdown_up.png"
                                    : "qrc:/assets/symboles/dropdown_down.png"
                            sourceSize.height: 24
                            fillMode: Image.PreserveAspectFit
                            visible: false
                        }

                        MultiEffect {
                            source: genresArrow
                            anchors.fill: genresArrow
                            brightness: 1.0
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: genres.genresOpen = !genres.genresOpen
                        }
                    }
                    
                    // CONTENT
                    Column {
                        width: parent.width
                        spacing: 5
                        visible: genres.genresOpen
                        
                        Text { text: "Horror"; color: "white"; font.pointSize: 15 }
                        Text { text: "Simulation"; color: "white"; font.pointSize: 15 }
                        Text { text: "Action"; color: "white"; font.pointSize: 15 }
                    }
                }
            }
        }
        //main part
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