#ifndef ACCESS_H
#define ACCESS_H

#include <QObject>
#include "videowidget.h"

class Access : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool vedioStop READ vedioStop WRITE setVedioStop NOTIFY vedioStopChanged)

public:
    Access(QObject *parent=Q_NULLPTR);
    ~Access();

    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();

    bool vedioStop();
    void setVedioStop(const bool &vedioStop);

signals:
    void vedioStopChanged();

private:
    // 通过这个窗口去启动视频窗口
    // 视频窗口为此窗口的子窗口
    QPointer<VideoWidget> m_mainVideoWidget;
    bool m_vedioStop = false;
};

#endif // ACCESS_H
