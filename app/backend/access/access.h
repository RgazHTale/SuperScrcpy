#ifndef ACCESS_H
#define ACCESS_H

#include <QObject>
#include "decoder.h"
#include "server.h"
#include "frames.h"

class Access : public QObject
{
public:
    Access();

private:
    server m_server;
    Decoder m_decoder;
    Frames m_frames;
};

#endif // ACCESS_H
