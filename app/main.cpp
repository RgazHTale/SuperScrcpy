// explain: Jax
// 1.用push的命令启动adb，将scrcpy-sercer推送到手机
// 2.adb启动反向代理功能，即将手机数据通过网络转发给PC
// 3.电脑监听反向代理端口
// 4.启动scrcpy-sercer，用于采集数据
// 5.接收数据并渲染

#include <QApplication>

//嵌入widget需要使用QApplication这个库
//#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QDir>
#include <QDebug>

#include "access.h"

int main(int argc, char *argv[])
{    
    /**************************mycode***************************/

    // 配置环境变量
#ifdef Q_OS_WIN32
    qputenv("QTSCRCPY_ADB_PATH", "..\\..\\..\\thrid_party\\adb\\win\\adb.exe");
    qputenv("QTSCRCPY_SERVER_PATH", "..\\..\\..\\thrid_party\\scrcpy-server.jar");
#endif

    /**************************mycode***************************/
    // 注册C++类到qml类型系统中
    qmlRegisterType<Access>("Access", 1, 0, "Access");
    /**************************mycode***************************/

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    /**************************mycode***************************/
    // 实例化资源管理器
    qDebug()<<"current applicationDirPath:"<<QCoreApplication::applicationDirPath();
    qDebug()<<"current currentPath:"<<QDir::currentPath();
    /**************************mycode***************************/

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    int ret = app.exec();

    Decoder::deInit();

    return ret;
}

