#include <QDebug>
#include <QTime>

#include "decoder.h"
#include "frames.h"
#include "devicesocket.h"

#define BUFSIZE 0x10000

Decoder::Decoder()
{

}

Decoder::~Decoder()
{

}

// 在调用所有函数之前都得调用这个初始化函数
bool Decoder::init()
{
    // 初始化
    if (avformat_network_init()) {
        return false;
    }
    return true;
}

// 这个函数来执行一些清理的操作
void Decoder::deInit()
{
    // 清理
    avformat_network_deinit();
}

void Decoder::setFrames(Frames *frames)
{
    // 保存解码帧
    m_frames = frames;
}

// 参数： Decoder对象，解码数据缓存，解码数据缓存大小
// 这里buf和bufSize应该是做了某种宏定义
// 这里的用法是直接调用read把数据读到buf中就行
// 返回读到的数据大小
static qint32 readPacket(void *opaque, quint8 *buf, qint32 bufSize) {
    Decoder *decoder = (Decoder*)opaque;
    if (decoder) {
        return decoder->recvData(buf, bufSize);
    }
    return 0;
}

void Decoder::setDeviceSocket(DeviceSocket* deviceSocket)
{
    // 把server中建立的套接字交给解码器
    m_deviceSocket = deviceSocket;
}

qint32 Decoder::recvData(quint8* buf, qint32 bufSize)
{
    if (!buf) {
        return 0;
    }
    if (m_deviceSocket) {
        // 从deviceSocket获取h264数据
        qint32 len = m_deviceSocket->subThreadRecvData(buf, bufSize);
        qDebug() << "recv data" << len;
        if (len == -1) {
            return AVERROR(errno);
        }
        if (len == 0) {
            return AVERROR_EOF;
        }
        return len;
    }
    return AVERROR_EOF;
}

// 开始解码
bool Decoder::startDecode()
{
    // 开始解码之前，需要先检查解码器是否已经获得了server的套接字
    // 所以在开始解码之前我们需要调用setDeviceSocket函数
    if (!m_deviceSocket) {
        return false;
    }
    m_quit = false;

    // start()函数将通过调用run()函数来开始线程的执行
    start();
    return true;
}

// 停止解码
void Decoder::stopDecode()
{
    m_quit = true;
    if (m_frames) {
        m_frames->stop();
    }
    // 等待解码线程退出
    wait();
}

void Decoder::run()
{
    // 临时缓冲区
    unsigned char *decoderBuffer = Q_NULLPTR;
    // io上下文：读取数据
    AVIOContext *avioCtx = Q_NULLPTR;
    // 封装上下文：解封装
    AVFormatContext *formatCtx = Q_NULLPTR;
    // 解码器：解码视频
    AVCodec *codec = Q_NULLPTR;
    // 解码器上下文：配合解码器
    AVCodecContext *codecCtx = Q_NULLPTR;
    // 清理标记
    // 封装上下文清理标记
    bool isFormatCtxOpen = false;
    // 解码上下文清理标记
    bool isCodecCtxOpen = false;

    // 申请解码缓冲区
    decoderBuffer = (unsigned char*)av_malloc(BUFSIZE);
    if (!decoderBuffer) {
        qCritical("Could not allocate buffer");
        goto runQuit;
    }

    // 初始化io上下文
    // 参数 ： 解码缓冲区，解码缓冲区大小，写标记（0即可），readPacket参数，数据读取回调函数
    // 把数据读到decoderBuffer
    avioCtx = avio_alloc_context(decoderBuffer, BUFSIZE, 0, this, readPacket, NULL, NULL);
    if (!avioCtx) {
        qCritical("Could not allocate avio context");
        av_free(decoderBuffer);
        goto runQuit;
    }

    // 初始化封装上下文
    formatCtx = avformat_alloc_context();
    if (!formatCtx) {
        qCritical("Could not allocate format context");
        goto runQuit;
    }
    // 为封装上下文指定io上下文
    // 将pb指针设置为之前设置的io上下文
    formatCtx->pb = avioCtx;
    // 打开封装上下文
    if (avformat_open_input(&formatCtx, NULL, NULL, NULL) < 0) {
        qCritical("Could not open video stream");
        goto runQuit;
    }
    isFormatCtxOpen = true;

    // 初始化解码器
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qCritical("H.264 decoder not found");
        goto runQuit;
    }

    // 初始化解码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        qCritical("Could not allocate decoder context");
        goto runQuit;
    }
    // 打开解码器上下文
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        qCritical("Could not open H.264 codec");
        goto runQuit;
    }
    isCodecCtxOpen = true;

    // 解码数据包：保存解码前的一帧h264数据
    AVPacket packet;// 我们不是读到数据就解码，而是当读取到的数据达到一帧时才解码，这个类就是用来保存解码前的一帧，打包数据
    // 初始化解码数据包
    av_init_packet(&packet);
    packet.data = Q_NULLPTR;
    packet.size = 0;

    // 从封装上下文中读取一帧解码前的数据，保存到AVPacket中
    // 注意解码视频的思路：
    // 先从io上下文中读取数据到封装上下文，封装上下文再将一帧数据给解码数据包
    while (!m_quit && !av_read_frame(formatCtx, &packet)) {
        // 获取AVFrame用来保存解码出来的yuv数据
        AVFrame* decodingFrame = m_frames->decodingFrame();
        // 解码
        int ret;
        // 解码h264
        // 这里应该是解码一个（一帧）packet之后把数据放到codecCtx中
        if ((ret = avcodec_send_packet(codecCtx, &packet)) < 0) {
            qCritical("Could not send video packet: %d", ret);
            goto runQuit;
        }
        // 取出yuv
        // 那么这里应该就是从codecCtx中取出一帧，保存在decodingFrame中
        if (decodingFrame) {
            ret = avcodec_receive_frame(codecCtx, decodingFrame);
        }
        if (!ret) {
            // 成功解码出一帧
            pushFrame();
        } else if (ret != AVERROR(EAGAIN)) {
            qCritical("Could not receive video frame: %d", ret);
            av_packet_unref(&packet);
            goto runQuit;
        }

        av_packet_unref(&packet);

        // eof_reached其实就是recvData()函数
        if (avioCtx->eof_reached) {
            break;
        }
    }
    qDebug() << "End of frames";

runQuit:
        if (avioCtx) {
            av_freep(&avioCtx);
        }
        if (formatCtx && isFormatCtxOpen) {
            avformat_close_input(&formatCtx);
        }
        if (formatCtx) {
            avformat_free_context(formatCtx);
        }
        if (codecCtx && isCodecCtxOpen) {
            avcodec_close(codecCtx);
        }
        if (codecCtx) {
            avcodec_free_context(&codecCtx);
        }

        emit onDecodeStop();
}

void Decoder::pushFrame()
{
    bool previousFrameConsumed = m_frames->offerDecodedFrame();
    if (!previousFrameConsumed) {
        return;
    }
    emit onNewFrame();
}
