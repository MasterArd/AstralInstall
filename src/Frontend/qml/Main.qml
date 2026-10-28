import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects


Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: "Astral"
    color: Colors.bgColor

    property string githubLoginName: ""
    property bool githubLoginPending: false
    property string githubLoginUrl: ""
    property string githubUserCode: ""

    Connections {
        target: backend

        function onLoginDeviceCodeReceived(url, userCode) {
            root.githubLoginUrl = url
            root.githubUserCode = userCode
            loginCodeDialog.open()
            if (!Qt.openUrlExternally(url))
                console.warn("Could not open GitHub login URL:", url)
        }

        function onLoginSucceeded(login) {
            root.githubLoginName = login
            root.githubLoginPending = false
            loginCodeDialog.close()
        }

        function onLoginFailed(error) {
            root.githubLoginPending = false
            loginCodeDialog.close()
            loginErrorDialog.text = error
            loginErrorDialog.open()
        }
    }

    Dialog {
        id: loginCodeDialog
        modal: true
        anchors.centerIn: parent
        width: 390
        padding: 20
        spacing: 16
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: Colors.panelColor
            border.color: Colors.lineColor
            border.width: 1
        }

        header: Rectangle {
            implicitHeight: 54
            color: "transparent"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.top: parent.top
                anchors.topMargin: 14
                text: "Sign in with GitHub"
                color: Colors.textColor
                font.pixelSize: 18
                font.bold: true
            }
        }

        contentItem: ColumnLayout {
            spacing: 14

            Text {
                text: "Open GitHub and enter this device code to authorize AstralInstall."
                color: Colors.textColor
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                Layout.maximumWidth: 350
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 70
                color: Colors.bgColor
                border.color: Colors.lineColor
                border.width: 1

                TextEdit {
                    anchors.centerIn: parent
                    text: root.githubUserCode
                    color: Colors.accentColor
                    font.pixelSize: 28
                    font.bold: true
                    font.letterSpacing: 2
                    readOnly: true
                    selectionColor: Colors.lineColor
                    selectByMouse: true
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                BusyIndicator {
                    running: root.githubLoginPending
                    implicitWidth: 22
                    implicitHeight: 22
                    palette.highlight: Colors.accentColor
                }
                Text {
                    text: "Waiting for approval on GitHub..."
                    color: Colors.accentColor
                    font.pixelSize: 13
                }
            }
            Text {
                text: "This sign-in attempt expires after 30 minutes."
                color: Colors.mutedColor
                font.pixelSize: 11
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 2
            }
        }

        footer: Item {
            implicitHeight: 56

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.bottomMargin: 10
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }
                Button {
                    id: openGithubButton
                    text: "Open GitHub"
                    onClicked: Qt.openUrlExternally(root.githubLoginUrl)

                    contentItem: Text {
                        text: openGithubButton.text
                        color: Colors.bgColor
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: openGithubButton.hovered ? "#A4D0B2" : Colors.accentColor
                        implicitWidth: 120
                        implicitHeight: 36
                    }
                }
                Button {
                    id: closeLoginDialogButton
                    text: "Close"
                    onClicked: loginCodeDialog.close()

                    contentItem: Text {
                        text: closeLoginDialogButton.text
                        color: Colors.textColor
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: closeLoginDialogButton.hovered ? Colors.lineColor : Colors.bgColor
                        border.color: Colors.lineColor
                        border.width: 1
                        implicitWidth: 90
                        implicitHeight: 36
                    }
                }
            }
        }
    }

    Dialog {
        id: loginErrorDialog
        property string text: ""
        modal: true
        anchors.centerIn: parent
        width: 360
        padding: 20
        spacing: 14

        background: Rectangle {
            color: Colors.panelColor
            border.color: Colors.lineColor
            border.width: 1
        }

        header: Rectangle {
            implicitHeight: 54
            color: "transparent"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.top: parent.top
                anchors.topMargin: 14
                text: "GitHub sign-in failed"
                color: Colors.textColor
                font.pixelSize: 18
                font.bold: true
            }
        }

        contentItem: Text {
            text: loginErrorDialog.text
            color: Colors.textColor
            wrapMode: Text.Wrap
            Layout.maximumWidth: 320
        }

        footer: Item {
            implicitHeight: 56

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.bottomMargin: 10

                Button {
                    id: loginErrorOkButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight
                    onClicked: loginErrorDialog.close()

                    contentItem: Text {
                        text: loginErrorOkButton.text
                        color: Colors.bgColor
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: loginErrorOkButton.hovered ? "#A4D0B2" : Colors.accentColor
                        implicitWidth: 90
                        implicitHeight: 36
                    }
                }
            }
        }
    }

    // Top bar
    Rectangle {
        id: navBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: 44
        color: Colors.panelColor

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Colors.lineColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10

            Button {
                id: libraryButton
                text: "library"

                Layout.preferredWidth: 100
                Layout.fillHeight: true

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true

                background: Rectangle {
                    color: "transparent"

                    // Active page marker
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        color: Colors.accentColor
                        visible: gameManager.currentPage === 0
                    }
                }
                contentItem: Text {
                    text: libraryButton.text
                    color: gameManager.currentPage === 0
                           ? Colors.textColor
                           : libraryButton.hovered ? Colors.accentColor : Colors.mutedColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 15
                    font.bold: gameManager.currentPage === 0

                    Behavior on color { ColorAnimation { duration: 120 } }
                }

                onClicked: gameManager.currentPage = 0
            }
            Button {
                id: myGamesButton
                text: "My games"

                Layout.preferredWidth: 100
                Layout.fillHeight: true

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true

                background: Rectangle {
                    color: "transparent"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 2
                        color: Colors.accentColor
                        visible: gameManager.currentPage === 1
                    }
                }
                contentItem: Text {
                    text: myGamesButton.text
                    color: gameManager.currentPage === 1
                           ? Colors.textColor
                           : myGamesButton.hovered ? Colors.accentColor : Colors.mutedColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 15
                    font.bold: gameManager.currentPage === 1

                    Behavior on color { ColorAnimation { duration: 120 } }
                }

                onClicked: gameManager.currentPage = 1
            }
            //gh login
            Button {
                id: githubButton
                text: root.githubLoginPending
                      ? "Waiting..."
                      : root.githubLoginName === "" ? "GitHub" : root.githubLoginName

                Layout.preferredWidth: 100
                Layout.preferredHeight: 26
                Layout.alignment: Qt.AlignVCenter

                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                flat: true
                enabled: !root.githubLoginPending || root.githubUserCode !== ""

                background: Rectangle {
                    color: Colors.bgColor
                    border.color: githubButton.hovered ? Colors.accentColor : Colors.lineColor
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
                contentItem: Row {
                    spacing: 6
                    leftPadding: 10
                    Image {
                        source: "qrc:/assets/symboles/Githublogo.png"
                        sourceSize.width: 14
                        sourceSize.height: 14
                        width: 14
                        height: 14
                        fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: githubButton.text
                        color: Colors.accentColor
                        font.pixelSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                onClicked: {
                    if (root.githubLoginPending) {
                        loginCodeDialog.open()
                    } else {
                        root.githubUserCode = ""
                        root.githubLoginUrl = ""
                        root.githubLoginPending = true
                        backend.login()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            TextField {
                id: searchField
                Layout.preferredWidth: 200
                Layout.preferredHeight: 26
                Layout.alignment: Qt.AlignVCenter
                color: Colors.textColor
                placeholderText: "Search..."
                placeholderTextColor: Colors.mutedColor
                font.pixelSize: 12
                leftPadding: 10
                rightPadding: 10

                background: Rectangle {
                    color: Colors.bgColor
                    border.color: searchField.activeFocus ? Colors.accentColor : Colors.lineColor
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
            }
            Item {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter

                Image {
                    id: dotsMask
                    anchors.fill: parent
                    source: "qrc:/assets/symboles/dots.png"
                    sourceSize.height: 20
                    fillMode: Image.PreserveAspectFit
                    visible: false
                }
                Rectangle {
                    anchors.fill: parent
                    color: dotsArea.containsMouse ? Colors.accentColor : Colors.mutedColor
                    layer.enabled: true
                    layer.effect: MultiEffect {
                        maskEnabled: true
                        maskSource: dotsMask
                    }

                    Behavior on color { ColorAnimation { duration: 120 } }
                }
                MouseArea {
                    id: dotsArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: testConsole.show()
                }
            }
        }
    }

    StackLayout {
        anchors.top: navBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: gameManager.currentPage

        Library { }
        MyGamesPage { }
    }
}
