import QtQuick
import QtQuick.Controls.Basic

Item {
    // Filled by StackLayout automatically — do NOT use anchors.fill: parent

    Rectangle { anchors.fill: parent; color: "#07142B" }

    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Compass"
            color: "#4FC3F7"
            font.pixelSize: 22
            font.bold: true
        }

        // ── Compass rose (pure QML rotation — no Canvas) ──────────────────────────
        Item {
            id: roseContainer
            anchors.horizontalCenter: parent.horizontalCenter
            width: 220; height: 220

            // Background circle
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "#0D1F3C"
                border.color: "#1E3A6E"
                border.width: 2
            }

            // Rotating rose — spins so N always points to magnetic north
            Item {
                anchors.centerIn: parent
                width: parent.width - 20
                height: parent.height - 20
                rotation: -sensors.azimuth

                Text {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "N"; color: "#E74C3C"
                    font.pixelSize: 22; font.bold: true
                }
                Text {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "S"; color: "#AABBCC"; font.pixelSize: 18
                }
                Text {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: "E"; color: "#AABBCC"; font.pixelSize: 18
                }
                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "W"; color: "#AABBCC"; font.pixelSize: 18
                }

                // N-S axis line
                Rectangle {
                    anchors.centerIn: parent
                    width: 3; height: parent.height - 36
                    color: "#334455"; radius: 1
                }
                // E-W axis line
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width - 36; height: 3
                    color: "#334455"; radius: 1
                }

                // Needle tip (red = north half)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height / 2 - 60
                    width: 8; height: 60
                    color: "#E74C3C"; radius: 3
                }
                // Needle tail (blue = south half)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height / 2
                    width: 8; height: 60
                    color: "#3498DB"; radius: 3
                }
                // Center pivot dot
                Rectangle {
                    anchors.centerIn: parent
                    width: 14; height: 14; radius: 7
                    color: "#FFFFFF"
                }
            }

            // Fixed top pointer
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 2
                width: 4; height: 16
                color: "#4FC3F7"; radius: 2
            }

            // Outer border ring
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                border.color: "#4FC3F7"
                border.width: 2
            }
        }

        // ── Readout ───────────────────────────────────────────────────────────────
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: sensors.cardinal
                color: "#E74C3C"; font.pixelSize: 48; font.bold: true
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: sensors.azimuthStr
                color: "#FFFFFF"; font.pixelSize: 24; font.bold: true
            }
        }
    }
}
