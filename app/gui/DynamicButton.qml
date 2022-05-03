import QtQuick 2.0
import AdbProcess 1.0

Rectangle {

    id: root
    width: 120
    height: 120
    radius: 60
    color: "#FFF8DC"
    border.color: "lightsteelblue"        //边框颜色
    border.width: 5                      //边框粗细

    Text {
        id: status
        text: "Start"
        color: "#2EB2B5"
        anchors.centerIn: parent
        font.family: "微软雅黑"
        font.pixelSize: 24
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked:{
            root.transStatuts()
        }
    }

//    AdbProcess {
//        id: adbprocess
//    }

    function transStatuts()
    {
        if(status.text == "Start"){
            status.text = "Stop"
            status.color = "#561238"
            //var path = adbprocess.push()
        }else{
            status.text = "Start"
            status.color = "#2EB2B5"
            // 这里将执行清理操作
        }
    }

}

