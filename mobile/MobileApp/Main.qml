import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.VirtualKeyboard

Window {
    id: window
    width: Screen.width
    height: Screen.height
    minimumWidth: 150
    minimumHeight: 250
    visible: true
    title: qsTr("Hello World")

    // Add a visible background
    Rectangle {
        anchors.fill: parent
        color: "steelblue"   // ← any color, just to confirm rendering works

        Text {
            anchors.centerIn: parent
            text: "Hello from Qt To Maz!"
            color: "white"
            font.pixelSize: 30
        }
    }

    InputPanel {
        id: inputPanel
        z: 99
        y: window.height
        width: window.width
        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                inputPanel.y: window.height - inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            NumberAnimation {
                properties: "y"
                easing.type: Easing.InOutQuad
            }
        }
    }
}