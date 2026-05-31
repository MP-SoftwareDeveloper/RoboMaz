import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    anchors.fill: parent
    color: "#1a1a2e"

    Column {
        anchors.centerIn: parent
        spacing: 40

        // Status indicator
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 120
            height: 120
            radius: 60
            color: flashlight.flashOn ? "#FFD700" : "#333355"

            Behavior on color {
                ColorAnimation { duration: 150 }
            }

            Text {
                anchors.centerIn: parent
                text: "🔦"
                font.pixelSize: 50
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: flashlight.flashOn ? "ON" : "OFF"
            color: flashlight.flashOn ? "#FFD700" : "#888888"
            font.pixelSize: 28
            font.bold: true
        }

        // Button 1 — Toggle ON/OFF
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 220
            height: 70
            radius: 12
            color: flashlight.flashOn ? "#c0392b" : "#27ae60"

            Behavior on color {
                ColorAnimation { duration: 150 }
            }

            Text {
                anchors.centerIn: parent
                text: flashlight.flashOn ? "Turn OFF" : "Turn ON"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: flashlight.toggle()
            }
        }

        // Button 2 — Blink 3 times
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 220
            height: 70
            radius: 12
            color: "#2980b9"

            Text {
                anchors.centerIn: parent
                text: "Blink 3×"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: flashlight.blinkThreeTimes()
            }
        }
    }
}