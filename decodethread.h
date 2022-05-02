#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <QMutex>

#include "decodeutils.h"

class DecodeThread : public QThread
{
    Q_OBJECT

public:

    DecodeThread();

    void Init(const QString& url);
    void SeekToPos(double sec, int flag);
    Thumbnail* GetThumbnail(int i);

    inline AVInfomation* GetAVInfomation() {
        return m_avInfomation;
    }

    inline QString GetErrorMsg() {
        return QString(m_errbuf);
    }

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
    void fillInfomation();
    void genThumbnails();
    void release();
    VideoFrame* decodeVideoFrame(AVFrame* frame);
    AudioFrame* decodeAudioFrame(AVFrame* frame);

private:

    std::mutex m_mutex;

    AVFormatContext* m_fmt_ctx = nullptr;
    AVCodecContext* m_video_codec_ctx = nullptr;
    AVCodecContext* m_audio_codec_ctx = nullptr;
    const AVCodec* m_video_codec = nullptr;
    const AVCodec* m_audio_codec = nullptr;
    int m_video_stream = -1;
    int m_audio_stream = -1;

    SwrContext* m_swr_ctx = nullptr;

    AVPacket* m_pkt = nullptr;
    AVFrame* m_frame = nullptr;

    bool m_close = true;
    std::atomic_bool m_exit = false;
    std::atomic_bool m_stop = false;

    AVInfomation* m_avInfomation = nullptr;

    double m_seek_pos = 0;

    char m_errbuf[1024];

    std::vector<Thumbnail*> m_thumbnails;

};

#endif // DECODETHREAD_H
