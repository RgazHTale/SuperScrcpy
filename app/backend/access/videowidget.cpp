#include "videowidget.h"

#define VIDEO_FROM_WIDTH 420
#define VIDEO_FROM_HEIGHT 850

VideoWidget::VideoWidget()
{
    // Lamada表达式捕获this指针，即可以调用这个类的成员变量
    connect(&m_server, &server::serverStartResult, this, [this](bool success){
       qDebug() << "server start" << success;
    });
    connect(&m_server, &server::connectToResult, this, [this](bool success, const QString& deviceName, const QSize& size){
       qDebug() << "connectToResult" << success << deviceName << size;
       updateShowSize(size);
       if(success){
           m_decoder.setDeviceSocket(m_server.getDeviceSocket());
           m_decoder.startDecode();

           m_inputConvert.setDeviceSocket(m_server.getDeviceSocket());
       }
    });

    m_frames.init();
    m_decoder.setFrames(&m_frames);

    // 主动关闭视频窗口也应该触发清理操作
    connect(&m_server, &server::onServerStop, this, [this](){
        m_videoWidget->close();
        qDebug() << "server process stop";
    });

    connect(&m_decoder, &Decoder::onDecodeStop, this, [this](){
        m_videoWidget->close();
        qDebug() << "decoder thread stop";
    });

    connect(&m_decoder, &Decoder::onNewFrame, this, [this](){
        qDebug() << "Decoder::onNewFrame";
        m_frames.lock();
        const AVFrame *frame = m_frames.consumeRenderedFrame();
        // 渲染frame

        // 发生窗口切换时更新宽高
        updateShowSize(QSize(frame->width, frame->height));

        m_videoWidget->setFrameSize(QSize(frame->width, frame->height));
        m_videoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2], frame->linesize[0], frame->linesize[1], frame->linesize[2]);

        m_frames.unLock();
    });

    // 这里不能传入this指针，因为QYUVOpenGLWidget()的参数是QWidget
    // 而且这里如果指定this为父对象的话它会在父对象上生成窗口，而不是一个单独的窗口
    m_videoWidget = new QYUVOpenGLWidget(this);
    m_videoWidget->resize(420, 850);
    // 由于父窗口没有设置大小，所以它的大小会有子窗口决定
    // 但是当子窗口确定了父窗口的大小之后
    // 父窗口的大小就确定了，不再会由子窗口发生改变
    // 所以要想改变父窗口，只能手动设置父窗口
    m_server.start("", 27183, 720, 8000000);
}

VideoWidget::~VideoWidget()
{
    // 我可以在这里实现一个信号
    // 只要调用析构函数，就
    m_server.stop();
    m_decoder.stopDecode();
    m_frames.deInit();

    // 发送信号，视频窗口已退出
    emit videoStop(true);
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    m_inputConvert.mouseEvent(event, m_videoWidget->frameSize(), m_videoWidget->size());
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_inputConvert.mouseEvent(event, m_videoWidget->frameSize(), m_videoWidget->size());
}

void VideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_inputConvert.mouseEvent(event, m_videoWidget->frameSize(), m_videoWidget->size());
}

void VideoWidget::closeEvent(QCloseEvent *e)
{
    emit videoStop(true);
    e->accept();//不会将事件传递给组件的父组件
    //e->ignore();// ignore函数会忽略事件，然后把事件传给父组件
}

bool VideoWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    PMSG msg = (PMSG)message;
      //监控设备改变信号
      if(msg->message == WM_DEVICECHANGE)
      {
        switch (msg->wParam) {
          //设备连接
          case DBT_DEVICEARRIVAL:
            qDebug() << "设备连接";
            break;
          //设备移出
          case DBT_DEVICEREMOVECOMPLETE:
            qDebug() << "设备移出";
            break;
          default:
            break;
        }
      }
      return QWidget::nativeEvent(eventType, message, result);
}

void VideoWidget::updateShowSize(const QSize &newSize)
{
    if (m_frameSize != newSize) {
        m_frameSize = newSize;

        bool vertical = newSize.height() > newSize.width();
        if (vertical) {
            m_videoWidget->resize(VIDEO_FROM_WIDTH, VIDEO_FROM_HEIGHT);
            resize(VIDEO_FROM_WIDTH, VIDEO_FROM_HEIGHT);
        } else {
            m_videoWidget->resize(VIDEO_FROM_HEIGHT, VIDEO_FROM_WIDTH);
            resize(VIDEO_FROM_HEIGHT, VIDEO_FROM_WIDTH);
        }
    }
}

