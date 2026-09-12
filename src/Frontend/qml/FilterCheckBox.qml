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
        implicitWidth: 16
        implicitHeight: 16
        x: control.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        color: control.checked ? Colors.panelColor : "transparent"
        border.width: 1
        border.color: control.checked || control.hovered ? Colors.accentColor : Colors.lineColor

        Behavior on border.color { ColorAnimation { duration: 120 } }

        // Check mark: two rotated bars instead of an image.
        Item {
            anchors.centerIn: parent
            width: 12
            height: 12
            visible: control.checked

            Rectangle {
                x: 1; y: 7.5
                width: 5; height: 2
                color: Colors.accentColor
                rotation: 45
                transformOrigin: Item.Center
            }
            Rectangle {
                x: 3.25; y: 5.5
                width: 9.5; height: 2
                color: Colors.accentColor
                rotation: -49
                transformOrigin: Item.Center
            }
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        text: control.text
        color: control.hovered ? Colors.textColor
             : control.checked ? Colors.textColor : Colors.mutedColor
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter

        Behavior on color { ColorAnimation { duration: 120 } }
    }
}
