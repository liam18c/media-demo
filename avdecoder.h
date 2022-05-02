#ifndef AVDECODER_H
#define AVDECODER_H

#include <QQueue>

#include <cmath>

#include "decodethread.h"

#define AV_SYNC_THRESHOLD_MIN 0.01
#define AV_SYNC_THRESHOLD_MAX 0.03
#define AV_FRAMEDOWN_TIME 0.01
#define AV_FRAMEUP_TIME 0.07

template <typename T>
class FrameQueue;

class AVDecoder : public QObject
{
    Q_OBJECT

    enum AVState {START = 0, STOP = 1, CLOSE = 2};
public:
    AVDecoder();
    //单例模式
    static AVDecoder* GetInstance() {
        static AVDecoder instance;
        return &instance;
    }

    void Open(QString url);
    //0代表绝对时间，1代表相对时间
    void SeekPos(double sec, int flag);
    //1代表顺序播放，-1代表倒放
    void SetPlayMode(int flag);
    void Close();

    //目前视频帧固定为rgb24的格式
    VideoFrame* GetVideoFrame();
    //目前音频帧全部重采样为44100，16，2格式
    AudioFrame* GetAudioFrame();

    void* operator new(size_t) = delete;

public slots:

signals:
    //解码环境初始化成功发送该信号，可捕获
    void Ready();

private slots:
    void receiveVideoFrame(VideoFrame* frame);
    void receiveAudioFrame(AudioFrame* frame);

    void onInit();

signals:
    void exit();
    void stop();
    void resume();

private:
    void resetClock();

    ~AVDecoder();

private: 
    std::atomic_int m_state = AVState::CLOSE;
    std::atomic_int m_play_mode = 1;

    QString m_url;
    DecodeThread* m_decode_thread = nullptr;

    int m_video_buffer_upper_size = 30;
    int m_video_buffer_lower_size = 15;

    std::mutex m_mutex;

    double m_seek_pos = 0;//sec
    double m_video_clock = 0;//sec
    double m_last_video_delay = 0;//sec
    double m_audio_clock = 0;//sec
    double m_last_audio_delay = 0;//sec

    FrameQueue<VideoFrame>* m_video_frame_queue;
    FrameQueue<AudioFrame>* m_audio_frame_queue;
};


template <typename T>
class FrameQueue {
public:
    void push(T* frame) {
        mutex.lock();
        data.enqueue(frame);
        mutex.unlock();
    }

    T* pop() {
        mutex.lock();
        if (data.size() == 0) {
            mutex.unlock();
            return nullptr;
        }
        T* frame = data.dequeue();
        mutex.unlock();
        return frame;
    }

    void clear() {
        mutex.lock();
        while (!data.empty()) {
            delete data.dequeue();
        }
        mutex.unlock();
    }

    int size() {
        mutex.lock();
        int temp = data.size();
        mutex.unlock();
        return temp;
    }

private:
    QMutex mutex;
    QQueue<T*> data;
};

#endif // AVDECODER_H
