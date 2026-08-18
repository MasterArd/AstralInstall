import QtQuick
import QtQuick.Controls

/*
    Ein Filtereintrag in der Sidebar: Checkbox + Label.
    Indikator und Haken sind gezeichnet, damit kein PNG noetig ist.
*/
CheckBox {
    id: control

    // Filter starten alle aktiv; einzelne Eintraege koennen mit
    // "checked: false" davon abweichen.
    checked: true

    padding: 0
    spacing: 8
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    // Kein Hover-/Klick-Highlight vom Controls-Style, nur unser Indikator.
    background: null

    indicator: Rectangle {
        id: box
        implicitWidth: 18
        implicitHeight: 18
        x: control.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        radius: 3
        color: control.checked ? Qt.lighter(Colors.color1, 1.6) : "transparent"
        border.width: 2
        border.color: control.checked || control.hovered ? "white" : "#a0a0a0"

        // Haken: zwei gedrehte Balken statt eines Bildes.
        Item {
            anchors.centerIn: parent
            width: 12
            height: 12
            visible: control.checked

            Rectangle {
                x: 1; y: 7.5
                width: 5; height: 2
                radius: 1
                color: "white"
                rotation: 45
                transformOrigin: Item.Center
            }
            Rectangle {
                x: 3.25; y: 5.5
                width: 9.5; height: 2
                radius: 1
                color: "white"
                rotation: -49
                transformOrigin: Item.Center
            }
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        text: control.text
        color: "white"
        font.pointSize: 15
        verticalAlignment: Text.AlignVCenter
    }
}
