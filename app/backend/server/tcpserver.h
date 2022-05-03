// 这个类只是为了方便server.cpp中获得套接字
// 而对QTcpServer进行的重写
// 因为m_deviceSocketde的类型是DeviceSocket
// 而nextPendingConnection()的返回值是QTcpSocket
// 所以需要重写一下

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    TcpServer(QObject *parent = Q_NULLPTR);

protected:
    virtual void incomingConnection(qintptr handle);
};

#endif // TCPSERVER_H
