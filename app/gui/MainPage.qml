import QtQml 2.0
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Page {

    DynamicButton {
        id: dynamicButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 50        
    }

    Rectangle {
        width: 150
        height: 3
        radius: 60
        border.color: "#D3D3D3"        //边框颜色
        border.width: 3
        anchors.right: phone.left
        anchors.top: dynamicButton.bottom
        anchors.topMargin: 44
    }

    Text {
        id: phone
        text: "📱"
        font.pixelSize: 30
        anchors.horizontalCenter: dynamicButton.horizontalCenter
        anchors.top: dynamicButton.bottom
        anchors.topMargin: 25
    }

    Rectangle {
        width: 150
        height: 3
        radius: 60
        border.color: "#D3D3D3"        //边框颜色
        border.width: 3
        anchors.left: phone.right
        anchors.top: dynamicButton.bottom
        anchors.topMargin: 44
    }

}
