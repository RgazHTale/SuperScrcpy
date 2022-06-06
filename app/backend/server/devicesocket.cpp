#include <QCoreApplication>
#include <QThread>
#include <QMutexLocker>

#include "devicesocket.h"
#include "qscrcpyevent.h"

DeviceSocket::DeviceSocket(QObject *parent)
    : QTcpSocket(parent)
{
    // 当有数据负载到网络套接字上时就会发出这个信号
    // read()函数可以从套接字中读取数据
    connect(this, &DeviceSocket::readyRead, this, &DeviceSocket::onReadyRead);
    connect(this, &DeviceSocket::disconnected, this, &DeviceSocket::quitNotify);
    connect(this, &DeviceSocket::aboutToClose, this, &DeviceSocket::quitNotify);
}

DeviceSocket::~DeviceSocket()
{

}

// 这是消费者的代码，外部调用的是这个
// 当套接字接受到数据之后会发出信号
// 这时候捕捉到信号，自动调用生产者
qint32 DeviceSocket::subThreadRecvData(quint8 *buf, qint32 bufSize)
{
    // 保证在子线程中调用
    Q_ASSERT(QCoreApplication::instance()->thread() != QThread::currentThread());
    if (m_quit) {
        return 0;
    }

    QMutexLocker locker(&m_mutex);

    m_buffer = buf;
    // 这个位置两个指针指向了同一位置
    // buf才是数据的接口
    // 此时m_buffer和buf指向了同一内存地址
    // 在生产者中，先是通过m_buffer指向同一位置修改了buf
    // 然后又让m_buffer指向空，断开了和之前内存数据的连接
    m_bufferSize = bufSize;
    m_dataSize = 0;

    // 发送事件
    DeviceSocketEvent* getDataEvent = new DeviceSocketEvent();
    QCoreApplication::postEvent(this, getDataEvent);

    while (!m_recvData) {
        m_recvDataCond.wait(&m_mutex);
    }

    m_recvData = false;
    return m_dataSize;
}

bool DeviceSocket::event(QEvent *event)
{
    if (event->type() == QScrcpyEvent::DeviceSocket) {
        onReadyRead();
        return true;
    }
    return QTcpSocket::event(event);
}

// 这是生产者的代码
// 收到负载数据的信号就开始生产，即把数据载入缓冲区中
void DeviceSocket::onReadyRead()
{
    QMutexLocker locker(&m_mutex);
    if (m_buffer && 0 < bytesAvailable()) { // 最开始m_buffer为空指针，跳出循环
        // 接收数据，要读多少数据
        qint64 readSize = qMin(bytesAvailable(), (qint64)m_bufferSize);

        // 由于这是在类的内部，而这个类又是继承于QTcpSocket
        // 所以可以直接调用read()函数
        // 因为它本身其实就是一个套接字
        // 调用read()就是调用自身的函数，所以不需要用套接字指针指向它就能调用
        m_dataSize = read((char*)m_buffer, readSize);// 读readSize大小的数据进缓冲区，返回本次读到的数据大小

        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_recvDataCond.wakeOne();
    }
}

void DeviceSocket::quitNotify()
{
    m_quit = true;
    QMutexLocker locker(&m_mutex);
    if (m_buffer) {
        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_dataSize = 0;
        m_recvDataCond.wakeOne();
    }
}
