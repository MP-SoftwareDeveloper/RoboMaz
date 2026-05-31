import QtQuick
import QtQuick.VirtualKeyboard

Window {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("RoboNet Mobile")

    FlashlightPage {
        anchors.fill: parent
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