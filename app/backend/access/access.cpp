#include <QDebug>

#include "access.h"

Access::Access(QObject *parent)
{
}

Access::~Access()
{
    // 手动释放内存
    delete m_mainVideoWidget;
}

void Access::startServer()
{
    m_mainVideoWidget = new VideoWidget();

    // m_mainVideoWidget必须new分配空间之后才能有意义
    connect(m_mainVideoWidget, &VideoWidget::videoStop, this, &Access::setVedioStop);

    // 窗口显示始终以父窗口为准
    // 如果子窗口超出父窗口，将被裁剪
    m_mainVideoWidget->show();
    m_mainVideoWidget->setAttribute(Qt::WA_DeleteOnClose);
}

void Access::stopServer()
{
    // 如果是点击叉叉退出，则会调用析构，m_mainVideoWidget就会为空
    // 若不是，那就调用析构
    if (m_mainVideoWidget){
        m_mainVideoWidget->close();
        delete m_mainVideoWidget;
    }
}

bool Access::vedioStop()
{
    return m_vedioStop;
}

void Access::setVedioStop(const bool &vedioStop)
{
    if (vedioStop == m_vedioStop)
        return;

    m_vedioStop = vedioStop;
    emit vedioStopChanged();
}

