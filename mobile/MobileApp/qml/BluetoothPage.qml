import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    // StackLayout sizes this item — do NOT use anchors.fill: parent
    color: "#07142B"

    // ── Header ────────────────────────────────────────────────────────────────────
    Column {
        id: header
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: 24
        }
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Bluetooth"
            color: "#4FC3F7"
            font.pixelSize: 22
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: bluetooth.status
            color: "#667A99"
            font.pixelSize: 14
        }
    }

    // ── Scan button ───────────────────────────────────────────────────────────────
    Rectangle {
        id: scanButton
        anchors {
            top: header.bottom
            topMargin: 20
            horizontalCenter: parent.horizontalCenter
        }
        width: 200
        height: 56
        radius: 28
        color: bluetooth.scanning ? "#C0392B" : "#1A6FBF"

        Behavior on color { ColorAnimation { duration: 200 } }

        Row {
            anchors.centerIn: parent
            spacing: 10

            // Animated scanning indicator
            Rectangle {
                visible: bluetooth.scanning
                width: 12; height: 12; radius: 6
                color: "#FFFFFF"
                anchors.verticalCenter: parent.verticalCenter

                SequentialAnimation on opacity {
                    running: bluetooth.scanning
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
            }

            Text {
                text: bluetooth.scanning ? "Stop Scan" : "Scan"
                color: "white"
                font.pixelSize: 20
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: bluetooth.scanning ? bluetooth.stopScan() : bluetooth.startScan()
        }
    }

    // ── Device list ───────────────────────────────────────────────────────────────
    Item {
        anchors {
            top: scanButton.bottom
            topMargin: 20
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: 16
            rightMargin: 16
            bottomMargin: 12
        }

        // Header row
        Row {
            id: listHeader
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 32
            spacing: 0

            Text {
                text: "⚡  Nearby Devices"
                color: "#4FC3F7"
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                text: bluetooth.devices.length > 0
                      ? bluetooth.devices.length + " found"
                      : ""
                color: "#667A99"
                font.pixelSize: 13
            }
        }

        // Divider
        Rectangle {
            id: divider
            anchors { top: listHeader.bottom; topMargin: 4; left: parent.left; right: parent.right }
            height: 1
            color: "#1E3A6E"
        }

        // Empty state
        Column {
            anchors.centerIn: parent
            spacing: 12
            visible: bluetooth.devices.length === 0 && !bluetooth.scanning

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "📡"
                font.pixelSize: 48
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Tap Scan to discover\nnearby Bluetooth devices"
                color: "#445566"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Scanning animation (when list is empty and scanning)
        Column {
            anchors.centerIn: parent
            spacing: 16
            visible: bluetooth.devices.length === 0 && bluetooth.scanning

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "🔍"
                font.pixelSize: 48

                SequentialAnimation on rotation {
                    running: bluetooth.scanning
                    loops: Animation.Infinite
                    NumberAnimation { from: -15; to: 15; duration: 800; easing.type: Easing.InOutSine }
                    NumberAnimation { from:  15; to: -15; duration: 800; easing.type: Easing.InOutSine }
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Scanning…"
                color: "#4FC3F7"
                font.pixelSize: 16
            }
        }

        // Device list
        ListView {
            anchors {
                top: divider.bottom
                topMargin: 4
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            model: bluetooth.devices
            clip: true
            spacing: 2

            delegate: Rectangle {
                required property string modelData
                required property int    index

                width: ListView.view.width
                height: 60
                color: index % 2 === 0 ? "#0D1F3C" : "#0F2244"
                radius: 8

                Row {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 16
                        rightMargin: 16
                    }
                    spacing: 12

                    // BT icon
                    Text {
                        text: "📶"
                        font.pixelSize: 22
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // Device name
                    Text {
                        text: modelData
                        color: "#DDEEFF"
                        font.pixelSize: 15
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                        width: parent.width - 40
                    }
                }
            }
        }
    }
}
