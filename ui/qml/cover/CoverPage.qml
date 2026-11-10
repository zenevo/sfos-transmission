import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Column {
        anchors.centerIn: parent
        spacing: Theme.paddingSmall
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Transmission")
            font.pixelSize: Theme.fontSizeMedium
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: backend.running ? qsTr("Running") : qsTr("Stopped")
            font.pixelSize: Theme.fontSizeExtraSmall
            color: backend.running ? Theme.highlightColor : Theme.secondaryColor
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("%1 %2").arg(backend.downSpeed).arg(backend.upSpeed)
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
        }
    }
}
