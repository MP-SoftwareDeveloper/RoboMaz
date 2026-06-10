import QtQuick
import QtQuick.Controls.Basic

Item {
    // Filled by StackLayout automatically — do NOT use anchors.fill: parent

    Rectangle { anchors.fill: parent; color: "#07142B" }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Tilt Sensor"
            color: "#4FC3F7"
            font.pixelSize: 22
            font.bold: true
        }

        // ── Artificial horizon (pure QML — no Canvas) ─────────────────────────────
        Item {
            id: horizonView
            anchors.horizontalCenter: parent.horizontalCenter
            width: 240; height: 240
            clip: true

            // Horizon world: rotates by roll, shifts by pitch
            Item {
                // Zero-size pivot at the center of horizonView
                x: horizonView.width  / 2
                y: horizonView.height / 2
                width: 0; height: 0

                rotation: -sensors.roll

                // Sky block (above horizon)
                Rectangle {
                    x: -horizonView.width * 2
                    y: -horizonView.height * 2 - sensors.pitch * 2
                    width: horizonView.width * 4
                    height: horizonView.height * 2
                    color: "#1A4480"
                }

                // Ground block (below horizon)
                Rectangle {
                    x: -horizonView.width * 2
                    y: -sensors.pitch * 2
                    width: horizonView.width * 4
                    height: horizonView.height * 2
                    color: "#5C3310"
                }

                // Horizon line
                Rectangle {
                    x: -horizonView.width * 2
                    y: -sensors.pitch * 2 - 1
                    width: horizonView.width * 4
                    height: 2
                    color: "#FFFFFF"
                }
            }

            // Fixed aircraft reference lines
            Rectangle {
                anchors.centerIn: parent
                width: 80; height: 3
                x: parent.width / 2 - 70
                color: "#FFD700"
            }
            Rectangle {
                anchors.centerIn: parent
                width: 80; height: 3
                x: parent.width / 2 - 10
                color: "#FFD700"
            }
            Rectangle {
                anchors.centerIn: parent
                width: 10; height: 10; radius: 5
                color: "#FFD700"
            }

            // Circular border overlay (drawn on top, clips the square look)
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                border.color: "#4FC3F7"
                border.width: 3
                z: 10
            }
        }

        // ── Numeric readout ───────────────────────────────────────────────────────
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 48

            Column {
                spacing: 4
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "PITCH"
                    color: "#667A99"; font.pixelSize: 11; font.bold: true
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: sensors.pitchStr
                    color: "#4FC3F7"; font.pixelSize: 30; font.bold: true
                }
            }

            Rectangle {
                width: 1; height: 54; color: "#1E3A6E"
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                spacing: 4
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "ROLL"
                    color: "#667A99"; font.pixelSize: 11; font.bold: true
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: sensors.rollStr
                    color: "#F4A261"; font.pixelSize: 30; font.bold: true
                }
            }
        }
    }
}
