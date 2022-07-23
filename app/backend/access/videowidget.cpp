#include "videowidget.h"

VideoWidget::VideoWidget()
{
    // Lamada表达式捕获this指针，即可以调用这个类的成员变量
    connect(&m_server, &server::serverStartResult, this, [this](bool success){
       qDebug() << "server start" << success;
    });
    connect(&m_server, &server::connectToResult, this, [this](bool success, const QString& deviceName, const QSize& size){
       qDebug() << "connectToResult" << success << deviceName << size;
       if(success){
           m_decoder.setDeviceSocket(m_server.getDeviceSocket());
           m_decoder.startDecode();
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

        m_videoWidget->setFrameSize(QSize(frame->width, frame->height));
        m_videoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2], frame->linesize[0], frame->linesize[1], frame->linesize[2]);

        m_frames.unLock();
    });

    // 这里不能传入this指针，因为QYUVOpenGLWidget()的参数是QWidget
    // 而且这里如果指定this为父对象的话它会在父对象上生成窗口，而不是一个单独的窗口
    m_videoWidget = new QYUVOpenGLWidget(Q_NULLPTR);
    m_videoWidget->resize(420, 850);
}

VideoWidget::~VideoWidget()
{
    m_server.stop();
    m_decoder.stopDecode();
    m_frames.deInit();
}
