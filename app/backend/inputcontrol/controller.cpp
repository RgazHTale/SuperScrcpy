#include <QCoreApplication>

#include "controller.h"
#include "devicesocket.h"
#include "controlevent.h"

Controller::Controller(QObject *parent)
    : QObject(parent)
{

}

void Controller::setDeviceSocket(DeviceSocket *deviceSocket)
{
    m_deviceSocket = deviceSocket;
}

void Controller::postControlEvent(ControlEvent *controlEvent)
{
    if (controlEvent) {
        // postEvent的参数：事件接收者、事件
        QCoreApplication::postEvent(this, controlEvent);
        // 也就是说接收到ControlEvent类型的事件后
        // 将调用event函数
    }
}

void Controller::test(QRect rc)
{
    ControlEvent* mouseEvent = new ControlEvent(ControlEvent::CET_MOUSE);
    mouseEvent->setMouseEventData(AMOTION_EVENT_ACTION_DOWN, AMOTION_EVENT_BUTTON_PRIMARY, rc);
    postControlEvent(mouseEvent);
}

bool Controller::event(QEvent *event)
{
    if (event && event->type() == ControlEvent::Control) {
        ControlEvent* controlEvent = dynamic_cast<ControlEvent*>(event);
        if (controlEvent) {
            sendControl(controlEvent->serializeData());
        }
        return true;
    }
    return QObject::event(event);
}

bool Controller::sendControl(const QByteArray &buffer)
{
    // 检查发送的数据是否为空，若为空则返回false
    if (buffer.isEmpty()) {
        return false;
    }
    qint32 len = 0;
    if (m_deviceSocket) {
        // 通过write函数向手机端发送数据
        // len为发送数据的长度
        len = m_deviceSocket->write(buffer.data(), buffer.length());
    }
    // 如果返回的数据长度和发送的数据长度相等，则表示发送成功
    return len == buffer.length() ? true : false;
}
