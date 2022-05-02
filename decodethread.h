#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <QMutex>

#include "decodeutils.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
}

struct AVInfomation {
    double duration = 0;

    int frame_rate = 0;

};


class DecodeThread : public QThread
{
    Q_OBJECT

public:
    DecodeThread();

    void Init(const QString& url);
    void SeekToPos(float sec);

    inline QString GetErrorMsg();
public slots:
    void Exit();
    void Stop();
    void Resume();

signals:
    void sendVideoFrame(VideoFrame* frame);
    void sendAudioFrame(AudioFrame* frame);

    void initSucceed();

protected:
    void run() override;

private:
    void release();

private:

    QMutex m_mutex;

    AVFormatContext* m_fmt_ctx = nullptr;
    AVCodecContext* m_video_codec_ctx = nullptr;
    AVCodecContext* m_audio_codec_ctx = nullptr;
    const AVCodec* m_video_codec = nullptr;
    const AVCodec* m_audio_codec = nullptr;
    AVPacket* m_pkt = nullptr;
    AVFrame* m_frame = nullptr;
    int m_video_stream = -1;
    int m_audio_stream = -1;

    SwrContext* m_swr_ctx = nullptr;

    std::atomic_bool m_exit = false;
    std::atomic_bool m_stop = false;

    AVInfomation* m_avInfomation = nullptr;

    char m_errbuf[1024];

};

#endif // DECODETHREAD_H
