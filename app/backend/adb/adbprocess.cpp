#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <iostream>

#include "adbprocess.h"

QString AdbProcess::s_adbPath = "";

AdbProcess::AdbProcess(QObject *parent)
    : QProcess(parent)
{
    // ..\thrid_party\adb\win\adb.exe
    initSignals();
}

QString AdbProcess::getAdbPath()
{
    if (s_adbPath.isEmpty()) {
        // 通过qgetenv获取qt的环境变量
        // fromLocal8Bit用来实现中文转码，避免中文路径无法识别
        s_adbPath = QString::fromLocal8Bit(qgetenv("QTSCRCPY_ADB_PATH"));
        QFileInfo fileInfo(s_adbPath);
        if (s_adbPath.isEmpty() || !fileInfo.isFile()) {
            // 当前程序所在目录
            // 对这里保持疑问，这句语句是否正确？
            // 回答：首先QCoreApplication::applicationDirPath()返回的就是以/拼接的路径
            //s_adbPath = QCoreApplication::applicationDirPath() + "/adb";
            // 这里给不给文件后缀名都行，不过为了严谨我还是给后缀名
            s_adbPath = QCoreApplication::applicationDirPath() + "/adb.exe";
        }
    }
    return s_adbPath;
}

// 向指定设备发送命令
void AdbProcess::execute(const QString &serial, const QStringList &args)
{
    QStringList adbArgs;
    if(!serial.isEmpty()) {
        adbArgs << "-s" << serial;
    }
    adbArgs << args;
    qDebug() << getAdbPath() << adbArgs.join(" ");
    start(getAdbPath(), adbArgs);
}

void AdbProcess::push(const QString &serial, const QString &local, const QString &remote)
{
    QStringList adbArgs;
    adbArgs << "push";
    adbArgs << local;
    adbArgs << remote;
    execute(serial, adbArgs);// 这个函数是专门用来执行命令的
}

void AdbProcess::removePath(const QString &serial, const QString &path)
{
    QStringList adbArgs;
    adbArgs << "shell";
    adbArgs << "rm";
    adbArgs << path;
    execute(serial, adbArgs);
}

void AdbProcess::reverse(const QString &serial, const QString &deviceSocketName, quint16 localPort)
{
    QStringList adbArgs;
    adbArgs << "reverse";
    adbArgs << QString("localabstract:%1").arg(deviceSocketName);
    adbArgs << QString("tcp:%1").arg(localPort);
    execute(serial, adbArgs);
}

void AdbProcess::reverseRemove(const QString &serial, const QString &deviceSocketName)
{
    QStringList adbArgs;
    adbArgs << "reverse";
    adbArgs << "--remove";
    adbArgs << QString("localabstract:%1").arg(deviceSocketName);
    execute(serial, adbArgs);
}

QStringList AdbProcess::getDevicesSerialFromStdOut()
{
    // List of devices attached\r\nP7C0218510000537\tdevice\r\nP7C0218510000537\tdevice
    QStringList serials;

    // 以换行符来切割字符串，即有\r\n又有\n的原因是windows和linux的换行符不一样
    QStringList devicesInfoList = m_standardOutput.split(QRegExp("\r\n|\n"), QString::SkipEmptyParts);
    for(QString deviceInfo : devicesInfoList) {
        QStringList deviceInfos = deviceInfo.split(QRegExp("\t"), QString::SkipEmptyParts);
        if (2 == deviceInfos.count() && 0 == deviceInfos[1].compare("device")) {
            serials << deviceInfos[0];
        }
    }
    return serials;
}

QString AdbProcess::getDeviceIPFromStdOut()
{
    /*
     * 32: wlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
    inet 192.168.0.174/24 brd 192.168.0.255 scope global wlan0
       valid_lft forever preferred_lft forever
    */

    QString ip = "";
    QString strIPExp = "inet [\\d.]*";
    QRegExp ipRegExp(strIPExp, Qt::CaseInsensitive);
    if (ipRegExp.indexIn(m_standardOutput) != -1) {
        ip = ipRegExp.cap(0);
        ip = ip.right(ip.size() - 5);
    }
    return ip;
}

QString AdbProcess::getStdOut()
{
    return m_standardOutput;
}

QString AdbProcess::getErrorOut()
{
    return m_errorOutput;
}

// 对QProcess的执行结果进行处理
void AdbProcess::initSignals()
{
    connect(this, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error){
        if (QProcess::FailedToStart == error) {
            emit adbProcessResult(AER_ERROR_MISSING_BINARY);
        } else {
            emit adbProcessResult(AER_ERROR_START);
        }
        qDebug() << error;
    });

    connect(this, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          this, [this](int exitCode, QProcess::ExitStatus exitStatus){
        if (QProcess::NormalExit == exitStatus && 0 == exitCode) {
            emit adbProcessResult(AER_SUCCESS_EXEC);
        } else {
            emit adbProcessResult(AER_ERROR_EXEC);
        }
        qDebug() << exitCode << exitStatus;
    });

    connect(this, &QProcess::readyReadStandardError, this, [this](){
        m_errorOutput = QString::fromLocal8Bit(readAllStandardError()).trimmed();
        qDebug() << m_errorOutput;
    });

    connect(this, &QProcess::readyReadStandardOutput, this, [this](){
        m_standardOutput= QString::fromLocal8Bit(readAllStandardOutput()).trimmed();
        qDebug() << m_standardOutput;
    });

    connect(this, &QProcess::started, this, [this](){
        emit adbProcessResult(AER_SUCCESS_START);
    });
}
