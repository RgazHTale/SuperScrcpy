import QtQml 2.0
import QtQuick 2.0
import QtQuick.Window 2.12
import QtQuick.Controls 1.4

ApplicationWindow {
    width: 300
    height: 480
    visible: true
    title: qsTr("SuperScrcpy")

    Image {
        id:background
        anchors.fill: parent
        source: "qrc:/res/background.jpg"
    }

    Text {
        id: serial
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Wellcome to Scrcpy")
        color: "black"
        font.bold: true
        font.pixelSize:24
    }

    Button{
        id:stopServerButton
        text: "stop Link"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 100
        height: 50
        onClicked: {
            button.stopServerButton()
        }
    }
    Button{
        id:closeButton
        text: "close app"
        x:200
        y:39
        width: 100
        height: 50
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        onClicked: {
            Qt.quit()
        }
    }

    Rectangle {
        id:radio
        width: 92
        height: 92
        radius: 50
        border.color: "#2EB2B5"        //边框颜色
        border.width: 1                       //边框粗细
        anchors.centerIn: parent
        Text {
            id: content
            text: "Start"
            color: "#2EB2B5"
            anchors.centerIn: parent
            font.pixelSize: 24
        }
        MouseArea
        {
            anchors.fill: parent
            onClicked:{
                button.startServerButton()
            }
        }
    }
}

