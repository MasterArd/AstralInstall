pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: card

    property string name: ""
    property string banner: ""
    readonly property real s: Math.max(0.7, Math.min(2.5, card.width / 300))

    HoverHandler { id: cardHover }

    Rectangle {
        id: frame
        anchors.fill: parent
        anchors.margins: Math.round(8 * card.s)
        radius: Math.round(8 * card.s)
        color: Colors.panelColor
        border.color: cardHover.hovered ? Colors.accentColor : Colors.lineColor
        border.width: Math.max(1, Math.round(card.s))
        clip: true

        Behavior on border.color { ColorAnimation { duration: 120 } }

        Image {
            id: bannerImage
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: frame.height * 0.58
            source: card.banner
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            visible: status === Image.Ready
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: frame.height * 0.58
            color: Colors.bgColor
            visible: bannerImage.status !== Image.Ready

            Text {
                anchors.centerIn: parent
                text: "Kein Banner"
                color: Colors.mutedColor
                font.pixelSize: Math.round(13 * card.s)
            }
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: frame.height * 0.58 + Math.round(12 * card.s)
            anchors.leftMargin: Math.round(14 * card.s)
            anchors.rightMargin: Math.round(14 * card.s)
            text: card.name
            color: Colors.textColor
            font.pixelSize: Math.round(18 * card.s)
            font.bold: true
            elide: Text.ElideRight
        }
    }
}
