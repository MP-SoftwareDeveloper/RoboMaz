import QtQuick
import QtQuick.Controls.Basic
import QtQuick.VirtualKeyboard

Window {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("RoboNet Mobile")

    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"

        Column {
            anchors.centerIn: parent
            spacing: 40

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Flashlight Test"
                color: "white"
                font.pixelSize: 32
            }

            // Simple visible button
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200
                height: 80
                radius: 10
                color: "orange"

                Text {
                    anchors.centerIn: parent
                    text: "TOGGLE FLASH"
                    color: "black"
                    font.pixelSize: 24
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("Button clicked!")
                        flashlight.toggle()
                    }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Status: " + (flashlight.flashOn ? "ON" : "OFF")
                color: "yellow"
                font.pixelSize: 28
            }
        }
    }
}