import QtQml 2.0
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.0

ApplicationWindow {

    id: root

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "MainPage.qml"
    }

    header: ToolBar {

        id: toolBar

        ToolButton {
            id: toolButton
            width: toolBar.width / 8
            text: stackView.depth > 1 ? "\u25C0" : "\u2630" //depth代表栈中有多少页
            // 每次点击ItemDelegate只会压一个栈，button依旧是列表的样式
            // 但是两次点击ItemDelegate后会压入两栈，故button会变成返回符号

            // StackView压栈后自动呈现栈顶

            onClicked: {
                if (stackView.depth > 1) {
                    stackView.pop(stackView.initialItem)
                } else {
                    drawer.open()
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: root.width * 0.35
        height: root.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    stackView.push(settings)
                    drawer.close()
                }
            }

            ItemDelegate {
                text: qsTr("Help")
                width: parent.width
                onClicked: {
                    stackView.push("Help.qml")
                    drawer.close()
                }
            }

            ItemDelegate {
                text: qsTr("About")
                width: parent.width
                onClicked: {
                    stackView.push(aboutPage)
                    drawer.close()
                }
            }
        }

    }

    Component {
        id: settings
        Page {
            title: qsTr("Settings")

            Rectangle {
                anchors.fill: parent
                color: "blue"
            }

        }

    }

}

