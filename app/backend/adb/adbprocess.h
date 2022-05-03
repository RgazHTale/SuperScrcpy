//这个类用于封装adb，即封装外部程序

#ifndef ADBPROCESS_H
#define ADBPROCESS_H
#include <QProcess>

// 注册到qml类型系统中
// 必须要实例化之后才能使用
// 除非从main中推过去（在main中实例化在推送到qml中）
#include <QtQml/qqml.h>

class AdbProcess : public QProcess
{
    Q_OBJECT
public:
    // 枚举类型，显示adb启动状态
    enum ADB_EXEC_RESULT {
        AER_SUCCESS_START,          // 启动成功
        AER_ERROR_START,            // 启动失败
        AER_SUCCESS_EXEC,           // 执行成功
        AER_ERROR_EXEC,             // 执行失败
        AER_ERROR_MISSING_BINARY,   // 找不到文件
    };

    AdbProcess(QObject *parent = Q_NULLPTR);

    // qml将要调用的方法
    Q_INVOKABLE void push(const QString& serial, const QString& local, const QString& remote);

    void execute(const QString& serial, const QStringList& args);
    void removePath(const QString& serial, const QString& path);
    void reverse(const QString& serial, const QString& deviceSocketName, quint16 localPort);
    void reverseRemove(const QString& serial, const QString& deviceSocketName);
    QStringList getDevicesSerialFromStdOut();
    QString getDeviceIPFromStdOut();
    QString getStdOut();
    QString getErrorOut();

    // 得到adb.exe的路径
    // 具有static关键字的全局变量会在多源文件编译时隐藏
    // 不具备static关键字的全局变量在多源文件时处于同一名字空间
    static QString getAdbPath();

signals:
    // 信号：发射adb执行结果
    void adbProcessResult(ADB_EXEC_RESULT processResult);

private:
    void initSignals();

    static QString s_adbPath;
    QString m_standardOutput = "";
    QString m_errorOutput = "";
};

#endif // ADBPROCESS_H
