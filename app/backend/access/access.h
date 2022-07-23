#ifndef ACCESS_H
#define ACCESS_H

#include <QObject>
#include "videowidget.h"

class Access : public QObject
{
    Q_OBJECT

public:
    Access(QObject *parent=Q_NULLPTR);
    ~Access();

    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();

private:
    QPointer<VideoWidget> m_videoWidget;
};

#endif // ACCESS_H
