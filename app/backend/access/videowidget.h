#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "decoder.h"
#include "server.h"
#include "frames.h"
#include "qyuvopenglwidget.h"
#include "inputconvertnormal.h"
#include "inputconvertgame.h"
#include <Windows.h>
#include <Dbt.h>

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    // 构建之后第一步应该是设置窗口大小
    VideoWidget();

    ~VideoWidget();

signals:
    void videoStop(bool v);

protected:
    // 这几个类是实现QWidget类的虚函数
    // 实现这几个虚函数，当产生鼠标或键盘事件时调用这些函数
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    void closeEvent(QCloseEvent * e);

    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
    void updateShowSize(const QSize &newSize);

private:
    server m_server;
    Decoder m_decoder;
    Frames m_frames;
    QYUVOpenGLWidget *m_videoWidget;

    //InputConvertNormal m_inputConvert;
    InputConvertGame m_inputConvert;

    QSize m_frameSize;
};

#endif // VIDEOWIDGET_H
