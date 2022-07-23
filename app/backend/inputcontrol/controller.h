// 这个类用于发送控制指令

#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QPointer>

class DeviceSocket;
class ControlEvent;
class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent=Q_NULLPTR);

    // 获取套接字的接口，通过外部调用这个函数，从而设置它的套接字
    void setDeviceSocket(DeviceSocket *deviceSocket);
    void postControlEvent(ControlEvent* controlEvent);
    void test(QRect rc);

protected:
    // 实现这个虚函数，接收到事件后会调用这个函数
    bool event(QEvent *event);

private:
    bool sendControl(const QByteArray& buffer);

private:
    QPointer<DeviceSocket> m_deviceSocket;
};

#endif // CONTROLLER_H
