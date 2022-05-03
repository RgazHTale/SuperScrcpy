#ifndef DEVICESOCKET_H
#define DEVICESOCKET_H

#include <QTcpSocket>
#include <QWaitCondition>
#include <QMutex>

class DeviceSocket : public QTcpSocket
{
    Q_OBJECT
public:
    DeviceSocket(QObject *parent = Q_NULLPTR);
    ~DeviceSocket();

    // 这个函数将在子线程中被调用，用来获取视频数据
    // 即供解码器线程使用，用来获取视频数据
    qint32 subThreadRecvData(quint8* buf, qint32 bufSize);

protected:
    bool event(QEvent *event);

protected slots:
    void onReadyRead();
    void quitNotify();

private:
    // 锁
    QMutex m_mutex;
    QWaitCondition m_recvDataCond; // 条件变量

    // 标志
    bool m_recvData = false;
    bool m_quit = false;

    // 数据缓存
    quint8* m_buffer = Q_NULLPTR; // 缓冲区
    qint32 m_bufferSize = 0; // 缓冲区大小
    qint32 m_dataSize = 0; // 表示本次收到了多少数据
};

#endif // DEVICESOCKET_H
