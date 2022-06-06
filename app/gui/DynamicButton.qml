import QtQuick 2.0

import Server 1.0
import Decoder 1.0
import Frames 1.0

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

    Server {
        id: m_server
    }

    Decoder {
        id: m_decoder
    }

    Frames {
        id: m_frames
    }

    // 这个地方将setFrames()的代码进行修改
    Component.onCompleted: {
        m_frames.init()
        m_decoder.setFrames(&m_frames)
    }

    function transStatuts()
    {
        if(status.text == "Start"){
            status.text = "Stop"
            status.color = "#561238"
            m_server.start("", 27183, 720, 8000000)
        }else{
            status.text = "Start"
            status.color = "#2EB2B5"
            m_server.stop()
            m_frames.deInit()
            // 这里将执行清理操作
        }
    }

}

