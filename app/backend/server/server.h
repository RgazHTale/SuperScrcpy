// 启动server的流程
// 执行Start()，将m_serverStartStep设置为SSS_PUSH
// 成功推送之后，m_workProcess将发射成功推送的信号
// 这个信号会被this捕捉，然后将执行onWorkProcessResult()
// 然后在onWorkProcessResult()函数中，它会记录上一步执行的结果
// 并把状态机设置为下一步
// 当状态机执行到启动server时，需要先监听本地端口（反向代理就是把电脑作为服务器）
// 然后启动安卓端的服务
// 启动之后就建立的连接（相当于网络通信）
// 因为之前我们用套接字（qt之中专门为服务器封装的套接字，QTcpServer，这里就称套接字了）监听了本地端口，所以当连接建立时他会发出信号


#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QSize>

#include "adbprocess.h"
#include "devicesocket.h"
#include "tcpserver.h"

class server : public QObject
{
    Q_OBJECT

    enum SERVER_START_STEP {
        SSS_NULL,
        SSS_PUSH,
        SSS_ENABLE_REVERSE,
        SSS_EXECUTE_SERVER,
        SSS_RUNNING,
    };

public:
    server(QObject *parent=Q_NULLPTR);

    bool start(const QString& serial, quint16 localPort, quint16 maxSize, quint32 bitRate);
    void stop();
    DeviceSocket* getDeviceSocket();
signals:
    void serverStartResult(bool success);
    void connectToResult(bool success, const QString& deviceName, const QSize& size);
    void onServerStop();

private slots:
    void onWorkProcessResult(AdbProcess::ADB_EXEC_RESULT processResult);

private:
    bool startServerByStep();
    bool pushServer();
    bool removeServer();
    bool enableTunnelReverse();
    bool disableTunnelReverse();
    bool execute();
    QString getServerPath();
    bool readInfo(QString& deviceName, QSize& size);

private:
    QString m_serial = "";
    quint16 m_localPort = 0;
    quint16 m_maxSize = 0;
    quint32 m_bitRate = 0;

    SERVER_START_STEP m_serverStartStep = SSS_NULL;

    AdbProcess m_workProcess; // 组合关系，负责adb命令的执行
    AdbProcess m_serverProcess; // 因为app_process是阻塞的，所以要用一个单独的属性来操作

    QString m_serverPath = "";
    bool m_serverCopiedToDevice = false;
    bool m_enableReverse = false;

    TcpServer m_serverSocket;
    DeviceSocket* m_deviceSocket = Q_NULLPTR;
    // m_serverSocket建立连接，然后把这个连接交给m_deviceSocket
};

#endif // SERVER_H
