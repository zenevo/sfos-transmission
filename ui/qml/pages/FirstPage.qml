import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    Timer {
        interval: 2000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: backend.refresh()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: qsTr("Transmission Daemon") }

            TextSwitch {
                id: runSwitch
                text: qsTr("Daemon")
                description: backend.running ? qsTr("Running — RPC on port 9091")
                                            : qsTr("Stopped")
                checked: backend.running
                automaticCheck: false
                onClicked: backend.setRunning(!backend.running)
            }

            SectionHeader { text: qsTr("Status") }

            DetailItem {
                label: qsTr("State")
                value: backend.running ? qsTr("Running") : qsTr("Stopped")
            }
            DetailItem {
                label: qsTr("Version")
                value: backend.version.length > 0 ? backend.version : "—"
            }
            DetailItem {
                label: qsTr("Torrents")
                value: String(backend.torrentCount)
            }
            DetailItem {
                label: qsTr("Download")
                value: backend.running ? backend.downSpeed : "—"
            }
            DetailItem {
                label: qsTr("Upload")
                value: backend.running ? backend.upSpeed : "—"
            }
            DetailItem {
                label: qsTr("RPC")
                value: "127.0.0.1:9091"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: backend.lastError.length > 0
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.errorColor
                text: backend.lastError
            }
        }
    }
}
