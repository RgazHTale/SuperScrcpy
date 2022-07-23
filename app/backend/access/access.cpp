#include <QDebug>

#include "access.h"

Access::Access(QObject *parent)
{

}

Access::~Access()
{
    m_server.stop();
    m_decoder.stopDecode();
    m_frames.deInit();
    // 手动释放内存
    delete m_videoWidget;
}

void Access::startServer()
{
    m_server.start("", 27183, 720, 8000000);
    m_videoWidget->show();
}

void Access::stopServer()
{
    m_server.stop();
    m_videoWidget->close();
}

