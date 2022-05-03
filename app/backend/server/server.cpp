#include <QFileInfo>
#include <QCoreApplication>

#include "server.h"

// 推送到安卓的哪个目录
#define DEVICE_SERVER_PATH "/data/local/tmp/scrcpy-server.jar"

// 安卓端套接字的名称
#define SOCKET_NAME "scrcpy"
#define DEVICE_NAME_FIELD_LENGTH 64

server::server(QObject *parent)
    : QObject(parent)
{
    connect(&m_workProcess, &AdbProcess::adbProcessResult, this, &server::onWorkProcessResult);
    connect(&m_serverProcess, &AdbProcess::adbProcessResult, this, &server::onWorkProcessResult);
    connect(&m_serverSocket, &QTcpServer::newConnection, this, [this](){
        // 强制类型转换
        // 使nextPendingConnection()的返回值变成DeviceSocket类型的
        m_deviceSocket = dynamic_cast<DeviceSocket*>(m_serverSocket.nextPendingConnection());

        QString deviceName;
        QSize size;
        // devices name, size
        if (m_deviceSocket && m_deviceSocket->isValid() && readInfo(deviceName, size)) {
            disableTunnelReverse();
            removeServer();
            emit connectToResult(true, deviceName, size);
        } else {
            stop();
            emit connectToResult(false, deviceName, size);
        }
    });
}

bool server::start(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate)
{
    m_serial = serial;
    m_localPort = localPort;
    m_maxSize = maxSize;
    m_bitRate = bitRate;

    // start push server
    m_serverStartStep = SSS_PUSH;
    return startServerByStep();
}

void server::stop()
{
    if (m_deviceSocket) {
        m_deviceSocket->close();
        //m_deviceSocket->deleteLater();
    }

    m_serverProcess.kill();
    disableTunnelReverse();
    removeServer();
    m_serverSocket.close();
}

DeviceSocket *server::getDeviceSocket()
{
    return m_deviceSocket;
}

void server::onWorkProcessResult(AdbProcess::ADB_EXEC_RESULT processResult)
{
    if (sender() == &m_workProcess) {
        if (SSS_NULL != m_serverStartStep) {
            switch (m_serverStartStep) {
            case SSS_PUSH:
                if (AdbProcess::AER_SUCCESS_EXEC == processResult) {
                    m_serverCopiedToDevice = true;
                    m_serverStartStep = SSS_ENABLE_REVERSE;
                    startServerByStep();
                } else if (AdbProcess::AER_SUCCESS_START != processResult) {
                    qCritical("adb push failed");
                    m_serverStartStep = SSS_NULL;
                    emit serverStartResult(false);
                }
                break;
            case SSS_ENABLE_REVERSE:
                if (AdbProcess::AER_SUCCESS_EXEC == processResult) {
                    m_enableReverse = true;
                    m_serverStartStep = SSS_EXECUTE_SERVER;
                    startServerByStep();
                } else if (AdbProcess::AER_SUCCESS_START != processResult) {
                    qCritical("adb reverse failed");
                    m_serverStartStep = SSS_NULL;
                    // removeServer
                    removeServer();
                    emit serverStartResult(false);
                }
                break;
            default:
                break;
            }
        }
    }

    if (sender() == &m_serverProcess) {
        if (SSS_EXECUTE_SERVER == m_serverStartStep) {
            if (AdbProcess::AER_SUCCESS_START == processResult) {
                m_serverStartStep = SSS_RUNNING;
                emit serverStartResult(true);
            } else if (AdbProcess::AER_ERROR_START == processResult) {
                // disable reverse
                disableTunnelReverse();
                qCritical("adb shell start server failed");
                m_serverStartStep = SSS_NULL;
                // removeServer
                removeServer();
                emit serverStartResult(false);
            }
        } else if (SSS_RUNNING == m_serverStartStep) {
            m_serverStartStep == SSS_NULL;
            emit onServerStop();
        }
    }
}

bool server::startServerByStep()
{
    bool stepSuccess = false;
    // push, enable reverse, execute server
    if (SSS_NULL != m_serverStartStep) {
        switch (m_serverStartStep) {
        case SSS_PUSH:
            stepSuccess = pushServer();
            break;
        case SSS_ENABLE_REVERSE:
            stepSuccess = enableTunnelReverse();
            break;
        case SSS_EXECUTE_SERVER:
            m_serverSocket.setMaxPendingConnections(1);
            if (!m_serverSocket.listen(QHostAddress::LocalHost, m_localPort)) {
                qCritical(QString("Could not listen on port %1").arg(m_localPort).toStdString().c_str());
                m_serverStartStep == SSS_NULL;
                disableTunnelReverse();
                removeServer();
                emit serverStartResult(false);
                return false;
            }
            stepSuccess = execute();
            break;
        default:
            break;
        }
    }

    if (!stepSuccess) {
        emit serverStartResult(false);
    }

    return stepSuccess;
}

bool server::pushServer()
{
    m_workProcess.push(m_serial, getServerPath(), DEVICE_SERVER_PATH);
    return true;
}

bool server::removeServer()
{
    if (!m_serverCopiedToDevice) {
        return true;
    }
    m_serverCopiedToDevice = false;

    AdbProcess* adb = new AdbProcess();
    if (!adb) {
        return false;
    }
    connect(adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult){
        if (AdbProcess::AER_SUCCESS_START != processResult) {
            sender()->deleteLater();
        }
    });
    adb->removePath(m_serial, DEVICE_SERVER_PATH);
    return true;
}

bool server::enableTunnelReverse()
{
    m_workProcess.reverse(m_serial, SOCKET_NAME, m_localPort);
    return true;
}

bool server::disableTunnelReverse()
{
    if (!m_enableReverse) {
        return true;
    }
    m_enableReverse = false;

    AdbProcess* adb = new AdbProcess();
    if (!adb) {
        return false;
    }
    connect(adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult){
        if (AdbProcess::AER_SUCCESS_START != processResult) {
            sender()->deleteLater();
        }
    });
    adb->reverseRemove(m_serial, SOCKET_NAME);
    return true;
}

// 这里执行成功之后是不是就会发送“建立了新的连接”的信号？
// 回答：
// 这里的信号是由m_serverSocket发送的
// 对对对！！
// 这里很重要，如果没理解这里就会陷入疑惑
// 因为前面在启动server之前，
// 我们需要监听一个端口，而这个端口就是用m_serverSocket这个套接字去监听的
// （好像TcpServer不是一个套接字，反正就是给服务器用的东西）
bool server::execute()
{
    // adb shell CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 1080 2000000 false ""
    QStringList args;
    args << "shell";
    args << QString("CLASSPATH=%1").arg(DEVICE_SERVER_PATH);
    args << "app_process";
    args << "/";
    args << "com.genymobile.scrcpy.Server";
    args << QString::number(m_maxSize);
    args << QString::number(m_bitRate);
    args << "false";
    args << "";

    // 启动服务
    m_serverProcess.execute(m_serial, args);
    return true;
}

QString server::getServerPath()
{
    if (m_serverPath.isEmpty()) {
        m_serverPath = QString::fromLocal8Bit(qgetenv("QTSCRCPY_SERVER_PATH"));
        QFileInfo fileInfo(m_serverPath);
        if (m_serverPath.isEmpty() || !fileInfo.isFile()) {
            m_serverPath = QCoreApplication::applicationDirPath() + "/scrcpy-server.jar";
        }
    }
    return m_serverPath;
}


// 成功建立连接之后安卓设备将发送一段数据过来
// 这段数据中包含着设备的一些信息
// readInfo()函数将读取并解释这段信息
bool server::readInfo(QString &deviceName, QSize &size)
{
    // abk001-----------------------0x0438 0x02d0
    //               64b            2b w   2b h
    unsigned char buf[DEVICE_NAME_FIELD_LENGTH + 4];
    if (m_deviceSocket->bytesAvailable() <= (DEVICE_NAME_FIELD_LENGTH + 4)) {
        m_deviceSocket->waitForReadyRead(300);
    }

    // read()函数：从设备读取最多maxSize字节到数据，并返回读取的字节数。
    qint64 len = m_deviceSocket->read((char*)buf, sizeof(buf));
    if (len < DEVICE_NAME_FIELD_LENGTH + 4) {
        qInfo("Could not retrieve device information");
        return false;
    }
    buf[DEVICE_NAME_FIELD_LENGTH - 1] = '\0';
    deviceName = (char*)buf;
    size.setWidth((buf[DEVICE_NAME_FIELD_LENGTH] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 1]);
    size.setHeight((buf[DEVICE_NAME_FIELD_LENGTH + 2] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 3]);
    return true;
}
