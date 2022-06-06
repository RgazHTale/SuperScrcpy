#include "access.h"

Access::Access()
{
    m_frames.init();
    m_decoder.setFrames(&m_frames);
}
