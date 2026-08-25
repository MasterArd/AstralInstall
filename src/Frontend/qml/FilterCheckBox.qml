import QtQuick
import QtQuick.Controls

/*
    A filter entry in the sidebar: checkbox + label.
    Indicator and check mark are drawn so that no PNG is needed.
*/
CheckBox {
    id: control

    // Filters all start active; individual entries can opt out with
    // "checked: false".
    checked: true

    padding: 0
    spacing: 8
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    // No hover/click highlight from the Controls style, only our indicator.
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

        // Check mark: two rotated bars instead of an image.
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
