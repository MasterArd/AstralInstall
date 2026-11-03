pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: card

    property string name: ""
    property string banner: ""
    property string version: ""
    property string repo: ""
    readonly property real s: Math.max(0.7, Math.min(2.5, card.width / 300))

    // only check on click, github has a limit per houe
    readonly property bool busy: installManager.busyGames.indexOf(card.name) !== -1
    property string newVersion: ""
    property bool checked: false
    property string status: ""
    property string error: ""

    Connections {
        target: installManager

        function onStatusChanged(name, status) {
            if (name === card.name)
                card.status = status
        }
        function onUpdateAvailable(name, version) {
            if (name === card.name) {
                card.newVersion = version
                card.checked = true
            }
        }
        function onUpToDate(name) {
            if (name === card.name) {
                card.newVersion = ""
                card.checked = true
            }
        }
        function onUpdateFinished(name) {
            if (name === card.name) {
                card.newVersion = ""
                card.status = ""
            }
        }
        function onInstallFailed(name, error) {
            if (name === card.name) {
                card.status = ""
                card.error = error
            }
        }
    }

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

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: Math.round(14 * card.s)
            spacing: Math.round(6 * card.s)

            Text {
                width: parent.width
                visible: card.error !== "" && !card.busy
                text: card.error
                color: "#E08A8A"
                font.pixelSize: Math.round(11 * card.s)
                elide: Text.ElideRight
            }

            Row {
                width: parent.width
                spacing: Math.round(8 * card.s)

                Text {
                    id: versionLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: card.version === "" ? "" : card.version.startsWith("v") ? card.version : "v" + card.version
                    color: Colors.mutedColor
                    font.pixelSize: Math.round(12 * card.s)
                }

                // hidden when from local zip (no repo)
                Rectangle {
                    id: updateButton
                    readonly property bool clickable: !card.busy
                    visible: card.repo !== ""
                    width: parent.width - versionLabel.width - parent.spacing
                    height: Math.round(28 * card.s)
                    color: clickable && updateHover.hovered ? Colors.lineColor : Colors.bgColor
                    border.color: card.newVersion !== "" ? Colors.accentColor : Colors.lineColor
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - Math.round(8 * card.s)
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        color: updateButton.clickable ? Colors.textColor : Colors.mutedColor
                        font.pixelSize: Math.round(12 * card.s)
                        font.bold: card.newVersion !== ""
                        text: card.busy ? (card.status !== "" ? card.status : "Suche...")
                            : card.newVersion !== "" ? "Update auf " + card.newVersion
                            : card.checked ? "Aktuell - erneut prüfen"
                            : "Nach Updates suchen"
                    }

                    HoverHandler {
                        id: updateHover
                        cursorShape: updateButton.clickable ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }

                    TapHandler {
                        enabled: updateButton.clickable
                        onTapped: {
                            card.error = ""
                            if (card.newVersion !== "")
                                installManager.update(card.name)
                            else
                                installManager.checkForUpdate(card.name)
                        }
                    }
                }
            }
        }
    }
}
