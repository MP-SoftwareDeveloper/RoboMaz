import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.VirtualKeyboard

Window {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("RoboMAZ")

    readonly property int tabH: 72
    property int currentTab: 0

    Rectangle { anchors.fill: parent; color: "#07142B" }

    // ── Pages (StackLayout — reliable sizing, no swipe conflicts) ─────────────────
    StackLayout {
        id: stack
        x: 0; y: 0
        width: window.width
        height: window.height - window.tabH
        currentIndex: window.currentTab

        FlashlightPage {}
        GyroscopePage  {}
        CompassPage    {}
        BluetoothPage  {}
    }

    // ── Tab bar ───────────────────────────────────────────────────────────────────
    Rectangle {
        x: 0
        y: window.height - window.tabH
        width: window.width
        height: window.tabH
        color: "#0D1F3C"

        Rectangle { width: parent.width; height: 1; color: "#1E3A6E" }

        Row {
            anchors.fill: parent

            Repeater {
                model: 4
                delegate: Item {
                    width: window.width / 4
                    height: window.tabH

                    Rectangle {
                        anchors.fill: parent
                        color: window.currentTab === index ? "#172A50" : "transparent"
                    }
                    Rectangle {
                        width: parent.width; height: 3
                        color: window.currentTab === index ? "#4FC3F7" : "transparent"
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 3

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: 22
                            text: index === 0 ? "🔦"
                                : index === 1 ? "📡"
                                : index === 2 ? "🧭"
                                :               "⚡"
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: 11
                            font.bold: window.currentTab === index
                            color: window.currentTab === index ? "#4FC3F7" : "#667A99"
                            text: index === 0 ? "Light"
                                : index === 1 ? "Gyro"
                                : index === 2 ? "Compass"
                                :               "BT"
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: window.currentTab = index
                    }
                }
            }
        }
    }

    // ── Virtual keyboard ──────────────────────────────────────────────────────────
    InputPanel {
        id: inputPanel
        z: 99
        y: window.height
        width: window.width
        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges { inputPanel.y: window.height - inputPanel.height }
        }
        transitions: Transition {
            from: ""; to: "visible"; reversible: true
            NumberAnimation { properties: "y"; easing.type: Easing.InOutQuad }
        }
    }
}
