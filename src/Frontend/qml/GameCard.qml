pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts

/*
 * One tile in the library grid. Library.qml fills the properties below
 * from the model, so the data side stays put no matter how this is
 * styled.
 */
Item {
    id: card

    property string name: ""
    property string description: ""
    property string developer: ""
    property string banner: ""
    property var platforms: []
    property var genres: []
    property string version: ""

    // Green theme palette - single place to retune the whole card.
    // Badges and chips share one look: panelColor filling, lineColor frame,
    // accentColor lettering.
    readonly property color bgColor: "#0D1410"
    readonly property color panelColor: "#101C14"
    readonly property color lineColor: "#25412E"
    readonly property color accentColor: "#8FBFA0"
    readonly property color textColor: "#E6F1E9"
    readonly property color mutedColor: "#7D9686"
    readonly property color borderColor: cardHover.hovered ? card.accentColor : card.lineColor

    Rectangle {
        id: frame
        anchors.fill: parent
        anchors.margins: 8
        color: card.bgColor
        border.color: card.borderColor
        border.width: 1

        Behavior on border.color { ColorAnimation { duration: 120 } }

        HoverHandler { id: cardHover }

        Item {
            id: bannerBox
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 1
            height: frame.height * 0.42
            clip: true

            Image {
                id: bannerImage
                width: bannerBox.width
                height: bannerBox.height + frame.radius
                source: card.banner
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                visible: false
                layer.enabled: true
            }

            Rectangle {
                id: bannerMask
                width: bannerImage.width
                height: bannerImage.height
                radius: frame.radius - 1
                visible: false
                layer.enabled: true
                layer.samples: 4
            }

            MultiEffect {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: bannerImage.height
                source: bannerImage
                maskEnabled: true
                maskSource: bannerMask
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
            }

            // Fade the banner into the card body
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 40
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 1.0; color: card.bgColor }
                }
            }
        }

        // Card body
        ColumnLayout {
            id: infoArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: bannerBox.bottom
            anchors.bottom: parent.bottom
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            anchors.topMargin: 4
            anchors.bottomMargin: 14
            spacing: 6

            // Name + version badge
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    color: card.textColor
                    text: card.name
                    font.pixelSize: 19
                    font.bold: true
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.preferredWidth: versionText.implicitWidth + 16
                    Layout.preferredHeight: 24
                    color: card.panelColor
                    border.color: card.lineColor
                    border.width: 1

                    Text {
                        id: versionText
                        anchors.centerIn: parent
                        color: card.accentColor
                        text: "v" + card.version
                        font.pixelSize: 12
                        font.bold: true
                    }
                }
            }

            // by @developer - genres
            Row {
                Layout.fillWidth: true
                spacing: 0

                Text {
                    id: byLabel
                    color: card.mutedColor
                    text: "by "
                    font.pixelSize: 12
                }

                Text {
                    id: devText
                    width: Math.min(implicitWidth, infoArea.width - byLabel.width - genreText.width)
                    color: devHover.hovered ? card.textColor : card.accentColor
                    text: "@" + card.developer
                    font.pixelSize: 12
                    elide: Text.ElideRight

                    HoverHandler { id: devHover }
                }

                Text {
                    id: genreText
                    color: card.mutedColor
                    visible: card.genres.length > 0
                    text: " \u00B7 " + card.genres.join(", ")
                    font.pixelSize: 12
                }
            }

            // Description
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 4
                color: card.mutedColor
                text: card.description
                font.pixelSize: 13
                lineHeight: 1.25
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                verticalAlignment: Text.AlignTop
            }

            // Platform chips
            Flow {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: card.platforms

                    Rectangle {
                        id: platformChip
                        required property string modelData

                        width: platformText.implicitWidth + 18
                        height: 24
                        color: card.panelColor
                        border.color: card.lineColor
                        border.width: 1

                        Text {
                            id: platformText
                            anchors.centerIn: parent
                            color: card.accentColor
                            text: platformChip.modelData
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }
}
