#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "decoder.h"
#include "server.h"
#include "frames.h"
#include "qyuvopenglwidget.h"

class VideoWidget : QWidget
{
public:
    VideoWidget();

    ~VideoWidget();

private:
    server m_server;
    Decoder m_decoder;
    Frames m_frames;
    QYUVOpenGLWidget *m_videoWidget;
};

#endif // VIDEOWIDGET_H
